# simple-https-server
This is a project, created to prove that writing an HTTP(S) Server is not the hardest in the world!

## Prerequisites
These are some libraries you'll need in order to build this project.
- `libthread`
- `libssl`
- `libjsoncpp`

## Generating a certificate
```bash
openssl req -x509 -nodes -new -sha256 -days 1024 -newkey rsa:2048 -keyout RootCA.key -out RootCA.pem -subj "/C=US/CN=Example-Root-CA"
openssl x509 -outform pem -in RootCA.pem -out RootCA.crt
```

## Have fun!!!