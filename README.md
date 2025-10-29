# Cliente e Servidor HTTP em C

Este projeto implementa um cliente (`client`) e um servidor (`server`) HTTP/1.1 básicos, desenvolvidos em C para a disciplina de Redes de Computadores.

## Recursos

### Servidor (`./server`)
* **Multi-thread**
* **Servidor de Arquivos Estáticos:** Serve arquivos de um diretório.
* **Listagem de Diretório:** Se um cliente requisitar um diretório que **não** contém um `index.html`, o servidor gera e envia dinamicamente uma página HTML com a lista de arquivos.
* **Tipos MIME:** Identifica e serve arquivos com os `Content-Type` corretos (HTML, JPG, PNG, TXT).
* **Tratamento de Erros:** Retorna `404 Not Found` para arquivos não encontrados.

### Cliente (`./client`)
* **Parse de URL:** Analisa URLs no formato `http://<host>:<port>/<path>`.
* **Resolução DNS:** Converte o *host* (ex: `www.ufsj.edu.br`) em um endereço IP.
* **Requisição HTTP:** Envia uma requisição `GET /path HTTP/1.1` válida.
* **Salvamento de Arquivo:** Se a resposta for `200 OK`, salva o corpo da resposta em um arquivo local (ex: `imagem.jpg`).
* **Tratamento de Erros:** Exibe mensagens de erro do servidor (ex: `404 Not Found`) no terminal.

## Compilação

O MakeFile oferece estes tipos de compilação:

```bash
# Compila tanto o cliente quanto o servidor
make all

# Compila apenas o servidor (gera o executável ./server)
make server

# Compila apenas o cliente (gera o executável ./client)
make client

# Limpa todos os arquivos compilados (executáveis e .o)
make clean
```

## Como Usar

### 1. Diretórios de Teste

O projeto inclui duas pastas de teste para o servidor:
* `data/`: Contém arquivos, mas **sem** `index.html` (para testar a listagem de diretório).
* `data_i/`: Contém arquivos **e** um `index.html` (para testar o serving padrão).

### 2. Iniciando o Servidor

Primeiro, compile e execute o servidor, apontando para um dos diretórios de dados.

```bash
# Compile o servidor
make server

# Execute o servidor (ex: servindo a pasta data/)
./server ./data
# O servidor estará rodando em http://localhost:8080
```
(Mude `8080` se a porta for diferente no seu código).

### 3. Usando o Cliente

Em **outro terminal**, compile e use o cliente para fazer requisições ao seu servidor ou a um site externo.

```bash
# Compile o cliente
make client

# Exemplo 1: Requisitar a raiz do seu servidor (sem index.html)
# Isso salvará um arquivo 'index.html' com a listagem do diretório.
./client http://localhost:8080/

# Exemplo 2: Requisitar um arquivo específico do seu servidor
# (Assumindo que 'imagem.jpg' existe na pasta 'data/')
./client http://localhost:8080/imagem.jpg
# Isso salvará o arquivo 'imagem.jpg' no diretório atual.
```

## Arquitetura e Funcionamento

### Pipeline do Servidor HTTP
1.  Recebe um diretório para servir (via `argv[1]`).
2.  Cria um socket e configura para reiniciar o servidor rapidamente.
3.  Faz `bind()` do socket à porta definida (8080).
4.  Entra em modo de escuta por conexões (máx: 10).
5.  Entra em um loop (`while(1)`):
    a.  Aguarda e aceita uma nova conexão de cliente.
    b.  **Cria uma `pthread` dedicada para lidar com a requisição do cliente.**
    c.  (A thread principal volta a esperar no `accept`).
7.  **Na Thread do Cliente:**
    a.  Recebe a requisição e verifica se é um `GET`.
    b.  Extrai o `path` (ex: `/imagem.jpg`) e o decodifica (ex: `%20` -> ` `).
    c.  **Lógica de Resposta:**
        i.  Se o `path` for `/`: Procura por `index.html`.
            - Se `index.html` existir, prepara para enviá-lo.
            - Se não, gera a listagem do diretório (`build_directory_listing`).
        ii. Se o `path` for um arquivo (ex: `/imagem.jpg`):
            - Verifica se o arquivo existe.
            - Se não existir, prepara uma resposta `404 Not Found`.
            - Se existir, prepara uma resposta `200 OK`.
    d.  Envia os cabeçalhos e o corpo (arquivo ou página 404) para o cliente.
    e.  Fecha o socket do cliente e a thread termina.

### Pipeline do Cliente HTT
1.  Recebe uma URL (ex: `http://localhost:8080/img.jpg`).
2.  Analisa (`parse_url`) a string, separando-a em:
    * `host`: "localhost"
    * `port`: "8080"
    * `path`: "/img.jpg"
3.  Usa `getaddrinfo` para converter o `host` e `port` em uma lista de endereços IP.
4.  Cria um `socket` e tenta se conectar ao primeiro endereço IP válido da lista.
5.  Formata uma string de requisição HTTP `GET`.
6.  Envia a requisição formatada ao servidor.
7.  Le primeiramente os **cabeçalhos** da resposta.
8.  **Verifica o Status:**
    * Se a primeira linha não for `200 OK` (ex: `404 Not Found`), imprime a linha de status e sai.
9.  **Salva o Arquivo:**
    * Extrai o nome do arquivo do `path` (ex: `img.jpg`).
    * Baixa e salva o corpo do arquivo, até que `recv` retorne 0 (conexão fechada pelo servidor).
10. Fecha o socket e o arquivo.

## Referências

https://dev.to/jeffreythecoder/how-i-built-a-simple-http-server-from-scratch-using-c-739
https://medium.com/@trish07/simple-steps-to-build-an-http-client-in-c-8e225b5c718c

## Licença

Este projeto é distribuído sob a licença MIT. Veja o arquivo `LICENSE` para mais detalhes.