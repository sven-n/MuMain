use clap::Parser;
use mu_protocol::ServerEntry;
use std::net::SocketAddr;
use std::str::FromStr;

use crate::server::ServerConfig;

#[derive(Debug, Clone, Parser, PartialEq, Eq)]
#[command(name = "mu_fake_server")]
pub struct Cli {
    #[arg(long, value_name = "ADDR")]
    pub bind: SocketAddr,

    #[arg(long = "server", value_name = "CONNECT_INDEX:PERCENT", required = true)]
    pub servers: Vec<ServerSpec>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ServerSpec {
    pub connect_index: u16,
    pub percent: u8,
}

impl ServerSpec {
    pub const fn new(connect_index: u16, percent: u8) -> Self {
        Self {
            connect_index,
            percent,
        }
    }
}

impl FromStr for ServerSpec {
    type Err = String;

    fn from_str(value: &str) -> Result<Self, Self::Err> {
        let (connect_index, percent) = value
            .split_once(':')
            .ok_or_else(|| "expected CONNECT_INDEX:PERCENT".to_string())?;

        let connect_index = connect_index
            .parse::<u16>()
            .map_err(|_| format!("invalid connect index: {connect_index}"))?;
        let percent = percent
            .parse::<u8>()
            .map_err(|_| format!("invalid percent: {percent}"))?;

        Ok(Self::new(connect_index, percent))
    }
}

impl From<ServerSpec> for ServerEntry {
    fn from(value: ServerSpec) -> Self {
        Self::new(value.connect_index, value.percent)
    }
}

impl Cli {
    pub fn into_config(self) -> ServerConfig {
        ServerConfig {
            bind_addr: self.bind,
            servers: self.servers.into_iter().map(Into::into).collect(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Cli;
    use clap::Parser;
    use std::net::SocketAddr;

    #[test]
    fn parses_bind_and_multiple_servers() {
        let cli = Cli::try_parse_from([
            "mu_fake_server",
            "--bind",
            "127.0.0.1:0",
            "--server",
            "0:100",
            "--server",
            "1:75",
        ])
        .unwrap();

        assert_eq!(cli.bind, "127.0.0.1:0".parse::<SocketAddr>().unwrap());
        assert_eq!(
            cli.servers,
            vec![
                super::ServerSpec::new(0, 100),
                super::ServerSpec::new(1, 75)
            ]
        );
    }
}
