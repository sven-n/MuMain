// mu_terrain_decrypt.cpp
// Decrypts Season16+ MU Online terrain files (ATT, MAP)
// Uses Crypto++ for ModulusDecrypt block ciphers.
//
// Usage: mu_terrain_decrypt <input_file> <output_file>
// Output is the raw decrypted binary data.
//
// Build: g++ -O2 -o mu_terrain_decrypt mu_terrain_decrypt.cpp -lcryptopp

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

#include <cryptopp/tea.h>
#include <cryptopp/3way.h>
#include <cryptopp/cast.h>
#include <cryptopp/rc5.h>
#include <cryptopp/rc6.h>
#include <cryptopp/mars.h>
#include <cryptopp/idea.h>
#include <cryptopp/gost.h>
#include <cryptopp/modes.h>

using BYTE = unsigned char;
using WORD = unsigned short;
using DWORD = unsigned int;

static const BYTE BUX_KEY[3] = { 0xFC, 0xCF, 0xAB };
static const BYTE MODULUS_KEY[33] = "webzen#@!01webzen#@!01webzen#@!0";

// Block cipher interface
struct ICipher {
    virtual ~ICipher() {}
    virtual int BlockSize() = 0;
    virtual void Decrypt(BYTE* in, size_t len, BYTE* out) = 0;
};

template<typename T>
struct CipherImpl : public ICipher {
    typename T::Decryption dec;
    int m_blockSize;

    bool Init(const BYTE* key) {
        try {
            dec.SetKey(key, T::DEFAULT_KEYLENGTH);
            m_blockSize = T::BLOCKSIZE;
            return true;
        } catch (...) {
            return false;
        }
    }

    int BlockSize() override { return m_blockSize; }

    void Decrypt(BYTE* in, size_t len, BYTE* out) override {
        size_t bs = m_blockSize;
        for (size_t i = 0; i + bs <= len; i += bs) {
            dec.ProcessBlock(in + i, out + i);
        }
    }
};

ICipher* MakeCipher(DWORD algorithm, const BYTE* key) {
    switch (algorithm & 7) {
        case 0: { auto c = new CipherImpl<CryptoPP::TEA>(); c->Init(key); return c; }
        case 1: { auto c = new CipherImpl<CryptoPP::ThreeWay>(); c->Init(key); return c; }
        case 2: { auto c = new CipherImpl<CryptoPP::CAST128>(); c->Init(key); return c; }
        case 3: { auto c = new CipherImpl<CryptoPP::RC5>(); c->Init(key); return c; }
        case 4: { auto c = new CipherImpl<CryptoPP::RC6>(); c->Init(key); return c; }
        case 5: { auto c = new CipherImpl<CryptoPP::MARS>(); c->Init(key); return c; }
        case 6: { auto c = new CipherImpl<CryptoPP::IDEA>(); c->Init(key); return c; }
        case 7: { auto c = new CipherImpl<CryptoPP::GOST>(); c->Init(key); return c; }
        default: return nullptr;
    }
}

bool ModulusDecrypt(std::vector<BYTE>& buf) {
    if (buf.size() < 34) return false;

    BYTE key_1[33];
    BYTE key_2[33] = {0};
    memcpy(key_1, MODULUS_KEY, 33);

    DWORD algorithm_1 = buf[1];
    DWORD algorithm_2 = buf[0];

    size_t size = buf.size();
    size_t data_size = size - 34;

    ICipher* cipher1 = MakeCipher(algorithm_1, key_1);
    if (!cipher1) return false;
    size_t block_size = 1024 - (1024 % cipher1->BlockSize());

    if (data_size > (4 * block_size)) {
        cipher1->Decrypt(&buf[2 + (data_size >> 1)], block_size, &buf[2 + (data_size >> 1)]);
    }
    if (data_size > block_size) {
        cipher1->Decrypt(&buf[size - block_size], block_size, &buf[size - block_size]);
        cipher1->Decrypt(&buf[2], block_size, &buf[2]);
    }
    delete cipher1;

    memcpy(key_2, &buf[2], 32);

    ICipher* cipher2 = MakeCipher(algorithm_2, key_2);
    if (!cipher2) return false;
    size_t block_size2 = data_size - (data_size % cipher2->BlockSize());
    cipher2->Decrypt(&buf[34], block_size2, &buf[34]);
    delete cipher2;

    buf.erase(buf.begin(), buf.begin() + 34);
    return true;
}

void Xor3Byte(BYTE* buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        buf[i] ^= BUX_KEY[i % 3];
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        fprintf(stderr, "Decrypts Season16+ EncTerrain ATT/MAP files.\n");
        fprintf(stderr, "Files must have ATT or MAP magic header.\n");
        return 1;
    }

    const char* input_path = argv[1];
    const char* output_path = argv[2];

    FILE* fin = fopen(input_path, "rb");
    if (!fin) { perror("fopen input"); return 1; }
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    std::vector<BYTE> raw(file_size);
    fread(raw.data(), 1, file_size, fin);
    fclose(fin);

    // Detect format by magic header
    bool is_att = false;
    bool is_map = false;

    if (file_size >= 4 && raw[0] == 'A' && raw[1] == 'T' && raw[2] == 'T' && raw[3] == 1) {
        is_att = true;
    } else if (file_size >= 4 && raw[0] == 'M' && raw[1] == 'A' && raw[2] == 'P' && raw[3] == 1) {
        is_map = true;
    } else {
        fprintf(stderr, "ERROR: File does not have ATT or MAP magic header\n");
        return 2;
    }

    // Strip 4-byte magic header
    std::vector<BYTE> body(raw.begin() + 4, raw.end());

    // Apply ModulusDecrypt
    if (!ModulusDecrypt(body)) {
        fprintf(stderr, "ERROR: ModulusDecrypt failed\n");
        return 1;
    }

    // Post-processing per original C++ source:
    //   ATT: ModulusDecrypt -> Xor3Byte
    //   MAP: ModulusDecrypt only
    if (is_att) {
        Xor3Byte(body.data(), body.size());
    }

    // Write output
    FILE* fout = fopen(output_path, "wb");
    if (!fout) { perror("fopen output"); return 1; }
    fwrite(body.data(), 1, body.size(), fout);
    fclose(fout);

    fprintf(stderr, "OK %zu\n", body.size());
    return 0;
}
