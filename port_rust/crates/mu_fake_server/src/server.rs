use mu_protocol::connect_server::{
    encode_server_list_response, is_server_list_request, ServerEntry,
};
use std::io::{self, Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::Arc;
use std::thread;

#[derive(Debug, Clone)]
pub struct ServerConfig {
    pub bind_addr: SocketAddr,
    pub servers: Vec<ServerEntry>,
}

pub fn run(config: ServerConfig) -> io::Result<()> {
    let listener = TcpListener::bind(config.bind_addr)?;
    let address = listener.local_addr()?;
    let response = Arc::<[u8]>::from(
        encode_server_list_response(&config.servers)
            .map_err(|error| io::Error::new(io::ErrorKind::InvalidInput, error))?,
    );

    println!("mu_fake_server listening on {address}");
    accept_loop(listener, response)
}

fn accept_loop(listener: TcpListener, response: Arc<[u8]>) -> io::Result<()> {
    for incoming in listener.incoming() {
        let stream = incoming?;
        let response = Arc::clone(&response);

        thread::spawn(move || {
            if let Err(error) = serve_connection(stream, response) {
                if !is_disconnect_error(&error) {
                    eprintln!("mu_fake_server connection failed: {error}");
                }
            }
        });
    }

    Ok(())
}

fn serve_connection(mut stream: TcpStream, response: Arc<[u8]>) -> io::Result<()> {
    send_packet(&mut stream, &response)?;

    while let Some(packet) = read_packet(&mut stream)? {
        if is_server_list_request(&packet) {
            send_packet(&mut stream, &response)?;
        }
    }

    Ok(())
}

fn read_packet(stream: &mut TcpStream) -> io::Result<Option<Vec<u8>>> {
    let mut code = [0u8; 1];
    match stream.read_exact(&mut code) {
        Ok(()) => {}
        Err(error) if error.kind() == io::ErrorKind::UnexpectedEof => {
            return Ok(None);
        }
        Err(error) => return Err(error),
    }

    let header_len = if code[0] % 2 == 1 { 4 } else { 5 };
    let mut header = vec![0u8; header_len - 1];
    stream.read_exact(&mut header)?;

    let size = if header_len == 4 {
        usize::from(header[0])
    } else {
        usize::from(u16::from_be_bytes([header[0], header[1]]))
    };

    if size < header_len {
        return Err(io::Error::new(
            io::ErrorKind::InvalidData,
            "packet header advertised a truncated size",
        ));
    }

    let mut packet = Vec::with_capacity(size);
    packet.push(code[0]);
    packet.extend_from_slice(&header);

    let body_len = size - header_len;
    let mut body = vec![0u8; body_len];
    if body_len > 0 {
        stream.read_exact(&mut body)?;
        packet.extend_from_slice(&body);
    }

    Ok(Some(packet))
}

fn send_packet(stream: &mut TcpStream, packet: &[u8]) -> io::Result<()> {
    stream.write_all(packet)?;
    stream.flush()
}

fn is_disconnect_error(error: &io::Error) -> bool {
    matches!(
        error.kind(),
        io::ErrorKind::BrokenPipe | io::ErrorKind::ConnectionReset | io::ErrorKind::UnexpectedEof
    )
}

#[cfg(test)]
mod tests {
    use super::{read_packet, serve_connection, ServerConfig};
    use crate::ServerEntry;
    use mu_protocol::connect_server::encode_server_list_response;
    use std::io::{self, Read, Write};
    use std::net::{SocketAddr, TcpListener, TcpStream};
    use std::sync::Arc;
    use std::thread;

    fn read_response(stream: &mut TcpStream) -> io::Result<Vec<u8>> {
        let mut code = [0u8; 1];
        stream.read_exact(&mut code)?;

        let header_len = if code[0] % 2 == 1 { 4 } else { 5 };
        let mut header = vec![0u8; header_len - 1];
        stream.read_exact(&mut header)?;

        let size = if header_len == 4 {
            usize::from(header[0])
        } else {
            usize::from(u16::from_be_bytes([header[0], header[1]]))
        };

        let mut packet = Vec::with_capacity(size);
        packet.push(code[0]);
        packet.extend_from_slice(&header);

        let mut body = vec![0u8; size - header_len];
        if !body.is_empty() {
            stream.read_exact(&mut body)?;
            packet.extend_from_slice(&body);
        }

        Ok(packet)
    }

    #[test]
    fn serves_server_list_on_connect_and_request() {
        let config = ServerConfig {
            bind_addr: "127.0.0.1:0".parse::<SocketAddr>().unwrap(),
            servers: vec![ServerEntry::new(7, 42)],
        };
        let response = Arc::<[u8]>::from(encode_server_list_response(&config.servers).unwrap());
        let listener = TcpListener::bind(config.bind_addr).unwrap();
        let address = listener.local_addr().unwrap();

        let server = thread::spawn({
            let response = Arc::clone(&response);
            move || {
                let (stream, _) = listener.accept().unwrap();
                serve_connection(stream, response).unwrap();
            }
        });

        let mut client = TcpStream::connect(address).unwrap();
        assert_eq!(read_response(&mut client).unwrap(), response.as_ref());

        client.write_all(&[0xC1, 0x04, 0xF4, 0x06]).unwrap();
        assert_eq!(read_response(&mut client).unwrap(), response.as_ref());

        drop(client);
        server.join().unwrap();
    }

    #[test]
    fn read_packet_returns_none_on_eof() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let address = listener.local_addr().unwrap();

        let server = thread::spawn(move || {
            let (stream, _) = listener.accept().unwrap();
            let mut stream = stream;
            assert!(read_packet(&mut stream).unwrap().is_none());
        });

        drop(TcpStream::connect(address).unwrap());
        server.join().unwrap();
    }
}
