use std::io::{self, BufRead, BufReader, Read, Write};
use std::net::{IpAddr, Ipv4Addr, Ipv6Addr, SocketAddr, TcpListener, TcpStream};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};

use crate::AppState;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ControlCommand {
    Boot,
    AssetCheckFailed,
    ReadyForLogin,
    Exit,
    Ping,
}

impl ControlCommand {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Boot => "boot",
            Self::AssetCheckFailed => "asset-check-failed",
            Self::ReadyForLogin => "ready-for-login",
            Self::Exit => "exit",
            Self::Ping => "ping",
        }
    }

    pub fn parse(value: &str) -> Option<Self> {
        match value.trim().to_ascii_lowercase().as_str() {
            "boot" => Some(Self::Boot),
            "asset-check-failed" | "asset_check_failed" => Some(Self::AssetCheckFailed),
            "ready-for-login" | "ready_for_login" => Some(Self::ReadyForLogin),
            "exit" => Some(Self::Exit),
            "ping" => Some(Self::Ping),
            _ => None,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ControlSnapshot {
    pub state: AppState,
    pub last_command: Option<ControlCommand>,
    pub command_count: u64,
}

impl ControlSnapshot {
    pub fn new(state: AppState) -> Self {
        Self {
            state,
            last_command: None,
            command_count: 0,
        }
    }

    pub fn apply_command(&mut self, command: ControlCommand) -> bool {
        self.last_command = Some(command);
        self.command_count = self.command_count.saturating_add(1);

        match command {
            ControlCommand::Boot => {
                self.state = AppState::Boot;
                false
            }
            ControlCommand::AssetCheckFailed => {
                self.state = AppState::AssetCheckFailed;
                false
            }
            ControlCommand::ReadyForLogin => {
                self.state = AppState::ReadyForLogin;
                false
            }
            ControlCommand::Exit => {
                self.state = AppState::Exit;
                true
            }
            ControlCommand::Ping => false,
        }
    }

    pub fn to_json(&self) -> String {
        let last_command = self
            .last_command
            .map(|command| format!("\"{}\"", command.as_str()))
            .unwrap_or_else(|| "null".to_string());

        format!(
            "{{\"state\":\"{}\",\"last_command\":{},\"command_count\":{}}}",
            self.state.as_str(),
            last_command,
            self.command_count
        )
    }
}

#[derive(Debug)]
pub struct ControlServerHandle {
    address: SocketAddr,
    snapshot: Arc<Mutex<ControlSnapshot>>,
    shutdown: Arc<AtomicBool>,
    join_handle: Option<JoinHandle<io::Result<()>>>,
}

impl ControlServerHandle {
    pub fn address(&self) -> SocketAddr {
        self.address
    }

    pub fn snapshot(&self) -> ControlSnapshot {
        self.snapshot
            .lock()
            .expect("control snapshot mutex poisoned")
            .clone()
    }

    pub fn request_shutdown(&self) {
        self.shutdown.store(true, Ordering::Release);
        let _ = poke_shutdown(self.address);
    }

    pub fn join(mut self) -> io::Result<ControlSnapshot> {
        let join_handle = self
            .join_handle
            .take()
            .expect("control server handle joined more than once");

        match join_handle.join() {
            Ok(result) => result?,
            Err(_) => {
                return Err(io::Error::other("control server thread panicked"));
            }
        }

        Ok(self.snapshot())
    }
}

impl Drop for ControlServerHandle {
    fn drop(&mut self) {
        if self.join_handle.is_some() {
            self.shutdown.store(true, Ordering::Release);
            let _ = poke_shutdown(self.address);
        }
    }
}

pub fn spawn(bind_addr: SocketAddr, initial_state: AppState) -> io::Result<ControlServerHandle> {
    let listener = TcpListener::bind(bind_addr)?;
    let address = listener.local_addr()?;
    let snapshot = Arc::new(Mutex::new(ControlSnapshot::new(initial_state)));
    let shutdown = Arc::new(AtomicBool::new(false));

    let thread_snapshot = Arc::clone(&snapshot);
    let thread_shutdown = Arc::clone(&shutdown);
    let join_handle = thread::spawn(move || serve_loop(listener, thread_snapshot, thread_shutdown));

    Ok(ControlServerHandle {
        address,
        snapshot,
        shutdown,
        join_handle: Some(join_handle),
    })
}

pub fn serve(bind_addr: SocketAddr, initial_state: AppState) -> io::Result<()> {
    let listener = TcpListener::bind(bind_addr)?;
    let snapshot = Arc::new(Mutex::new(ControlSnapshot::new(initial_state)));
    let shutdown = Arc::new(AtomicBool::new(false));

    serve_loop(listener, snapshot, shutdown)
}

fn serve_loop(
    listener: TcpListener,
    snapshot: Arc<Mutex<ControlSnapshot>>,
    shutdown: Arc<AtomicBool>,
) -> io::Result<()> {
    loop {
        if shutdown.load(Ordering::Acquire) {
            return Ok(());
        }

        let (stream, _) = listener.accept()?;
        handle_connection(stream, &snapshot, &shutdown)?;

        if shutdown.load(Ordering::Acquire) {
            return Ok(());
        }
    }
}

fn handle_connection(
    stream: TcpStream,
    snapshot: &Arc<Mutex<ControlSnapshot>>,
    shutdown: &Arc<AtomicBool>,
) -> io::Result<()> {
    let mut reader = BufReader::new(stream);
    let request = match read_request(&mut reader) {
        Ok(request) => request,
        Err(_) => {
            let mut stream = reader.into_inner();
            let _ = write_response(
                &mut stream,
                400,
                "Bad Request",
                "application/json",
                r#"{"error":"bad request"}"#,
            );
            return Ok(());
        }
    };

    let mut stream = reader.into_inner();
    let response = route_request(request, snapshot, shutdown);
    let _ = write_response(
        &mut stream,
        response.status,
        response.reason,
        response.content_type,
        &response.body,
    );
    Ok(())
}

fn route_request(
    request: HttpRequest,
    snapshot: &Arc<Mutex<ControlSnapshot>>,
    shutdown: &Arc<AtomicBool>,
) -> HttpResponse {
    match (request.method.as_str(), request.path.as_str()) {
        ("GET", "/") | ("GET", "/state") => {
            let snapshot = snapshot.lock().expect("control snapshot mutex poisoned");
            HttpResponse::json(200, "OK", snapshot.to_json())
        }
        ("POST", "/command") => {
            let command_name = request.command_name();

            let Some(command_name) = command_name else {
                return HttpResponse::json(
                    400,
                    "Bad Request",
                    r#"{"error":"missing command"}"#.to_string(),
                );
            };

            let Some(command) = ControlCommand::parse(command_name) else {
                return HttpResponse::json(
                    400,
                    "Bad Request",
                    r#"{"error":"unknown command"}"#.to_string(),
                );
            };

            let mut snapshot = snapshot.lock().expect("control snapshot mutex poisoned");
            let should_shutdown = snapshot.apply_command(command);
            let response = HttpResponse::json(200, "OK", snapshot.to_json());

            if should_shutdown {
                shutdown.store(true, Ordering::Release);
            }

            response
        }
        ("GET", "/__shutdown") => {
            shutdown.store(true, Ordering::Release);
            HttpResponse::empty(204, "No Content")
        }
        _ => HttpResponse::json(404, "Not Found", r#"{"error":"not found"}"#.to_string()),
    }
}

fn read_request(reader: &mut BufReader<TcpStream>) -> io::Result<HttpRequest> {
    let mut request_line = String::new();
    reader.read_line(&mut request_line)?;
    if request_line.trim().is_empty() {
        return Err(io::Error::new(
            io::ErrorKind::InvalidData,
            "missing request line",
        ));
    }

    let mut parts = request_line.split_whitespace();
    let method = parts
        .next()
        .ok_or_else(|| io::Error::new(io::ErrorKind::InvalidData, "missing request method"))?;
    let target = parts
        .next()
        .ok_or_else(|| io::Error::new(io::ErrorKind::InvalidData, "missing request target"))?;

    let mut content_length = 0usize;

    loop {
        let mut line = String::new();
        let bytes_read = reader.read_line(&mut line)?;
        if bytes_read == 0 {
            break;
        }

        let trimmed = line.trim_end_matches(['\r', '\n']);
        if trimmed.is_empty() {
            break;
        }

        if let Some((name, value)) = trimmed.split_once(':') {
            if name.trim().eq_ignore_ascii_case("content-length") {
                content_length = value.trim().parse().unwrap_or(0);
            }
        }
    }

    let mut body = vec![0; content_length];
    if content_length > 0 {
        reader.read_exact(&mut body)?;
    }

    let body = String::from_utf8(body)
        .map_err(|_| io::Error::new(io::ErrorKind::InvalidData, "request body is not utf-8"))?;

    let (path, query) = split_target(target);
    Ok(HttpRequest {
        method: method.to_string(),
        path,
        query,
        body,
    })
}

fn split_target(target: &str) -> (String, String) {
    let (path, query) = target.split_once('?').unwrap_or((target, ""));
    (path.to_string(), query.to_string())
}

fn poke_shutdown(address: SocketAddr) -> io::Result<()> {
    let mut stream = TcpStream::connect(wakeup_target(address))?;
    write_all(
        &mut stream,
        "GET /__shutdown HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",
    )?;
    let mut response = Vec::new();
    let _ = stream.read_to_end(&mut response);
    Ok(())
}

fn wakeup_target(address: SocketAddr) -> SocketAddr {
    if !address.ip().is_unspecified() {
        return address;
    }

    let ip = match address {
        SocketAddr::V4(_) => IpAddr::V4(Ipv4Addr::LOCALHOST),
        SocketAddr::V6(_) => IpAddr::V6(Ipv6Addr::LOCALHOST),
    };

    SocketAddr::new(ip, address.port())
}

fn write_response(
    stream: &mut TcpStream,
    status: u16,
    reason: &str,
    content_type: &str,
    body: &str,
) -> io::Result<()> {
    let body_bytes = body.as_bytes();
    let response = format!(
        "HTTP/1.1 {} {}\r\nContent-Type: {}\r\nContent-Length: {}\r\nConnection: close\r\n\r\n",
        status,
        reason,
        content_type,
        body_bytes.len()
    );

    write_all(stream, &response)?;
    write_all(stream, body)
}

fn write_all(stream: &mut TcpStream, body: &str) -> io::Result<()> {
    stream.write_all(body.as_bytes())?;
    stream.flush()
}

fn query_value<'a>(query: &'a str, key: &str) -> Option<&'a str> {
    query.split('&').find_map(|pair| {
        let (name, value) = pair.split_once('=')?;
        if name == key {
            Some(value)
        } else {
            None
        }
    })
}

#[derive(Debug, Clone)]
struct HttpRequest {
    method: String,
    path: String,
    query: String,
    body: String,
}

impl HttpRequest {
    fn command_name(&self) -> Option<&str> {
        query_value(&self.query, "name")
            .or_else(|| query_value(&self.body, "name"))
            .or_else(|| {
                let body = self.body.trim();
                if body.is_empty() {
                    None
                } else {
                    Some(body)
                }
            })
    }
}

#[derive(Debug, Clone)]
struct HttpResponse {
    status: u16,
    reason: &'static str,
    content_type: &'static str,
    body: String,
}

impl HttpResponse {
    fn json(status: u16, reason: &'static str, body: String) -> Self {
        Self {
            status,
            reason,
            content_type: "application/json",
            body,
        }
    }

    fn empty(status: u16, reason: &'static str) -> Self {
        Self {
            status,
            reason,
            content_type: "text/plain; charset=utf-8",
            body: String::new(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{spawn, ControlCommand, ControlSnapshot};
    use crate::AppState;
    use std::io::{Read, Write};
    use std::net::TcpStream;

    fn read_response(stream: &mut TcpStream) -> (String, String) {
        let mut response = String::new();
        stream.read_to_string(&mut response).unwrap();

        let (head, body) = response.split_once("\r\n\r\n").unwrap();
        (head.to_string(), body.to_string())
    }

    fn send_request(address: std::net::SocketAddr, request: &str) -> (String, String) {
        let mut stream = TcpStream::connect(address).unwrap();
        stream.write_all(request.as_bytes()).unwrap();
        stream.flush().unwrap();
        read_response(&mut stream)
    }

    #[test]
    fn snapshot_serializes_current_state() {
        let mut snapshot = ControlSnapshot::new(AppState::ReadyForLogin);
        snapshot.apply_command(ControlCommand::Ping);

        assert_eq!(
            snapshot.to_json(),
            r#"{"state":"ready-for-login","last_command":"ping","command_count":1}"#
        );
    }

    #[test]
    fn server_exposes_state_and_applies_commands() {
        let handle = spawn("127.0.0.1:0".parse().unwrap(), AppState::Boot).unwrap();
        let address = handle.address();

        let (_, body) = send_request(
            address,
            "GET /state HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",
        );
        assert!(body.contains(r#""state":"boot""#));

        let (_, body) = send_request(
            address,
            "POST /command?name=ready-for-login HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\nConnection: close\r\n\r\n",
        );
        assert!(body.contains(r#""state":"ready-for-login""#));
        assert!(body.contains(r#""last_command":"ready-for-login""#));

        let (_, body) = send_request(
            address,
            "POST /command?name=exit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\nConnection: close\r\n\r\n",
        );
        assert!(body.contains(r#""state":"exit""#));

        let final_snapshot = handle.join().unwrap();
        assert_eq!(final_snapshot.state, AppState::Exit);
        assert_eq!(final_snapshot.command_count, 2);
    }

    #[test]
    fn unknown_command_returns_error() {
        let handle = spawn("127.0.0.1:0".parse().unwrap(), AppState::Boot).unwrap();
        let address = handle.address();

        let (head, body) = send_request(
            address,
            "POST /command?name=unknown HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\nConnection: close\r\n\r\n",
        );

        assert!(head.contains("400 Bad Request"));
        assert!(body.contains(r#""error":"unknown command""#));

        handle.request_shutdown();
        let _ = handle.join();
    }

    #[test]
    fn shutdown_request_joins_cleanly() {
        let handle = spawn("127.0.0.1:0".parse().unwrap(), AppState::Boot).unwrap();

        handle.request_shutdown();

        let final_snapshot = handle.join().unwrap();
        assert_eq!(final_snapshot.state, AppState::Boot);
        assert_eq!(final_snapshot.command_count, 0);
    }

    #[test]
    fn shutdown_request_joins_cleanly_on_wildcard_bind() {
        let handle = spawn("0.0.0.0:0".parse().unwrap(), AppState::Boot).unwrap();

        handle.request_shutdown();

        let final_snapshot = handle.join().unwrap();
        assert_eq!(final_snapshot.state, AppState::Boot);
        assert_eq!(final_snapshot.command_count, 0);
    }
}
