#!/usr/bin/env python3
"""
ModulusCryptor decryption test for Season20 EncTerrain .obj files.

Implements all 8 cipher algorithms used by Webzen's ModulusCryptor:
  0: TEA       (block=8,  key=16 bytes, big-endian)
  1: ThreeWay  (block=12, key=12 bytes, little-endian)
  2: CAST5     (block=8,  key=16 bytes, big-endian)
  3: RC5       (block=8,  key=16 bytes, little-endian)
  4: RC6       (block=16, key=16 bytes, little-endian)
  5: MARS      (block=16, key=16 bytes, little-endian)
  6: IDEA      (block=8,  key=16 bytes, big-endian)
  7: GOST      (block=8,  key=32 bytes, little-endian)

Also implements FileCryptor and BuxCryptor XOR schemes.

Ported from the TypeScript implementations in:
  cpp/muonline-bmd-viewer-season-20/src/crypto/
"""

import struct
import sys
import os

# ============================================================
#  Utility
# ============================================================

def u32(x):
    """Mask to unsigned 32-bit."""
    return x & 0xFFFFFFFF

def rotl32(x, n):
    """Rotate left 32-bit."""
    n &= 31
    return u32((x << n) | (u32(x) >> (32 - n)))

def rotr32(x, n):
    """Rotate right 32-bit."""
    n &= 31
    return u32((u32(x) >> n) | (x << (32 - n)))

def mul32(a, b):
    """32-bit unsigned multiply (low 32 bits), matching Math.imul."""
    # Python ints are arbitrary precision, so just multiply and mask
    return u32(a * b)

def read_u32_le(buf, off):
    return struct.unpack_from('<I', buf, off)[0]

def write_u32_le(val, buf, off):
    struct.pack_into('<I', buf, off, u32(val))

def read_u32_be(buf, off):
    return struct.unpack_from('>I', buf, off)[0]

def write_u32_be(val, buf, off):
    struct.pack_into('>I', buf, off, u32(val))

def read_u16_be(buf, off):
    return struct.unpack_from('>H', buf, off)[0]

def write_u16_be(val, buf, off):
    struct.pack_into('>H', buf, off, val & 0xFFFF)


# ============================================================
#  TEA Cipher (algorithm 0)
#  Block=8, Key=16, Big-endian (matches BouncyCastle TeaEngine)
# ============================================================

class TEACipher:
    BLOCK_SIZE = 8
    DELTA = 0x9E3779B9
    ROUNDS = 32

    def __init__(self, key):
        key = key[:16]
        self.k = [read_u32_be(key, i * 4) for i in range(4)]

    def get_block_size(self):
        return self.BLOCK_SIZE

    def decrypt_block(self, src, src_off, dst, dst_off):
        v0 = read_u32_be(src, src_off)
        v1 = read_u32_be(src, src_off + 4)
        k0, k1, k2, k3 = self.k
        s = u32(self.DELTA * self.ROUNDS)  # 0xC6EF3720

        for _ in range(self.ROUNDS):
            v1 = u32(v1 - u32(u32(u32(v0 << 4) + k2) ^ u32(v0 + s) ^ u32((v0 >> 5) + k3)))
            v0 = u32(v0 - u32(u32(u32(v1 << 4) + k0) ^ u32(v1 + s) ^ u32((v1 >> 5) + k1)))
            s = u32(s - self.DELTA)

        write_u32_be(v0, dst, dst_off)
        write_u32_be(v1, dst, dst_off + 4)

    def block_decrypt(self, in_buf, length, out_buf):
        for i in range(0, length, self.BLOCK_SIZE):
            self.decrypt_block(in_buf, i, out_buf, i)


# ============================================================
#  ThreeWay Cipher (algorithm 1)
#  Block=12, Key=12, Little-endian
# ============================================================

class ThreeWayCipher:
    BLOCK_SIZE = 12
    ROUNDS = 11
    START_D = 0xB1B1

    def __init__(self, key):
        key = key[:12]
        self.k = [0, 0, 0]
        for i in range(3):
            self.k[i] = u32(
                (key[4*i + 3]) |
                (key[4*i + 2] << 8) |
                (key[4*i + 1] << 16) |
                (key[4*i] << 24)
            )
        # Transform key for decryption
        t = {'a0': self.k[0], 'a1': self.k[1], 'a2': self.k[2]}
        _tw_theta(t)
        _tw_mu(t)
        self.k[0] = _tw_reverse_bytes(t['a0'])
        self.k[1] = _tw_reverse_bytes(t['a1'])
        self.k[2] = _tw_reverse_bytes(t['a2'])

    def get_block_size(self):
        return self.BLOCK_SIZE

    def decrypt_block(self, src, src_off, dst, dst_off):
        t = {
            'a0': read_u32_le(src, src_off),
            'a1': read_u32_le(src, src_off + 4),
            'a2': read_u32_le(src, src_off + 8),
        }
        rc = self.START_D
        _tw_mu(t)
        for _ in range(self.ROUNDS):
            t['a0'] = u32(t['a0'] ^ self.k[0] ^ (rc << 16))
            t['a1'] = u32(t['a1'] ^ self.k[1])
            t['a2'] = u32(t['a2'] ^ self.k[2] ^ rc)
            _tw_rho(t)
            rc = u32(rc << 1)
            if rc & 0x10000:
                rc ^= 0x11011
            rc &= 0xFFFF
        t['a0'] = u32(t['a0'] ^ self.k[0] ^ (rc << 16))
        t['a1'] = u32(t['a1'] ^ self.k[1])
        t['a2'] = u32(t['a2'] ^ self.k[2] ^ rc)
        _tw_theta(t)
        _tw_mu(t)
        write_u32_le(t['a0'], dst, dst_off)
        write_u32_le(t['a1'], dst, dst_off + 4)
        write_u32_le(t['a2'], dst, dst_off + 8)

    def block_decrypt(self, in_buf, length, out_buf):
        for i in range(0, length, self.BLOCK_SIZE):
            self.decrypt_block(in_buf, i, out_buf, i)


def _tw_reverse_bytes(x):
    return u32(
        ((x & 0x000000FF) << 24) |
        ((x & 0x0000FF00) << 8) |
        ((x & 0x00FF0000) >> 8) |
        ((x & 0xFF000000) >> 24)
    )

def _tw_reverse_bits(a):
    a = u32(((a & 0xAAAAAAAA) >> 1) | ((a & 0x55555555) << 1))
    a = u32(((a & 0xCCCCCCCC) >> 2) | ((a & 0x33333333) << 2))
    return u32(((a & 0xF0F0F0F0) >> 4) | ((a & 0x0F0F0F0F) << 4))

def _tw_theta(t):
    c0 = u32(t['a0'] ^ t['a1'] ^ t['a2'])
    c = u32(rotl32(c0, 16) ^ rotl32(c0, 8))
    b0 = u32((t['a0'] << 24) ^ (t['a2'] >> 8) ^ (t['a1'] << 8) ^ (t['a0'] >> 24))
    b1 = u32((t['a1'] << 24) ^ (t['a0'] >> 8) ^ (t['a2'] << 8) ^ (t['a1'] >> 24))
    t['a0'] = u32(t['a0'] ^ c ^ b0)
    t['a1'] = u32(t['a1'] ^ c ^ b1)
    t['a2'] = u32(t['a2'] ^ c ^ u32((b0 >> 16) ^ (b1 << 16)))

def _tw_mu(t):
    t['a1'] = _tw_reverse_bits(t['a1'])
    tmp = _tw_reverse_bits(t['a0'])
    t['a0'] = _tw_reverse_bits(t['a2'])
    t['a2'] = tmp

def _tw_pi_gamma_pi(t):
    b2 = rotl32(t['a2'], 1)
    b0 = rotl32(t['a0'], 22)
    t['a0'] = rotl32(u32(b0 ^ (t['a1'] | u32(~b2))), 1)
    t['a2'] = rotl32(u32(b2 ^ (b0 | u32(~t['a1']))), 22)
    t['a1'] = u32(t['a1'] ^ (b2 | u32(~b0)))

def _tw_rho(t):
    _tw_theta(t)
    _tw_pi_gamma_pi(t)


# ============================================================
#  RC5 Cipher (algorithm 3)
#  Block=8, Key=16, Little-endian, w=32, r=16
# ============================================================

class RC5Cipher:
    BLOCK_SIZE = 8
    ROUNDS = 16
    P32 = 0xB7E15163
    Q32 = 0x9E3779B9

    def __init__(self, key):
        key = key[:16]
        self.S = self._expand_key(key)

    def get_block_size(self):
        return self.BLOCK_SIZE

    def _expand_key(self, key):
        r = self.ROUNDS
        c = max(len(key) // 4, 1)
        L = [0] * c
        for i in range(len(key) - 1, -1, -1):
            L[i // 4] = u32((L[i // 4] << 8) + key[i])

        s_len = 2 * (r + 1)
        S = [0] * s_len
        S[0] = self.P32
        for i in range(1, s_len):
            S[i] = u32(S[i - 1] + self.Q32)

        A = B = ii = jj = 0
        v = 3 * max(s_len, c)
        for _ in range(v):
            A = S[ii] = rotl32(u32(S[ii] + A + B), 3)
            B = L[jj] = rotl32(u32(L[jj] + A + B), (A + B) & 31)
            ii = (ii + 1) % s_len
            jj = (jj + 1) % c
        return S

    def decrypt_block(self, src, src_off, dst, dst_off):
        r = self.ROUNDS
        S = self.S
        A = read_u32_le(src, src_off)
        B = read_u32_le(src, src_off + 4)

        for i in range(r, 0, -1):
            B = u32(rotr32(u32(B - S[2 * i + 1]), A & 31) ^ A)
            A = u32(rotr32(u32(A - S[2 * i]), B & 31) ^ B)

        B = u32(B - S[1])
        A = u32(A - S[0])

        write_u32_le(A, dst, dst_off)
        write_u32_le(B, dst, dst_off + 4)

    def block_decrypt(self, in_buf, length, out_buf):
        for i in range(0, length, self.BLOCK_SIZE):
            self.decrypt_block(in_buf, i, out_buf, i)


# ============================================================
#  RC6 Cipher (algorithm 4)
#  Block=16, Key=16, Little-endian, w=32, r=20
# ============================================================

class RC6Cipher:
    BLOCK_SIZE = 16
    ROUNDS = 20
    P32 = 0xB7E15163
    Q32 = 0x9E3779B9

    def __init__(self, key):
        key = key[:16]
        self.S = self._expand_key(key)

    def get_block_size(self):
        return self.BLOCK_SIZE

    def _expand_key(self, key):
        r = self.ROUNDS
        c = max(len(key) // 4, 1)
        L = [0] * c
        for i in range(len(key) - 1, -1, -1):
            L[i // 4] = u32((L[i // 4] << 8) + key[i])

        s_len = 2 * r + 4
        S = [0] * s_len
        S[0] = self.P32
        for i in range(1, s_len):
            S[i] = u32(S[i - 1] + self.Q32)

        A = B = ii = jj = 0
        v = 3 * max(s_len, c)
        for _ in range(v):
            A = S[ii] = rotl32(u32(S[ii] + A + B), 3)
            B = L[jj] = rotl32(u32(L[jj] + A + B), (A + B) & 31)
            ii = (ii + 1) % s_len
            jj = (jj + 1) % c
        return S

    def decrypt_block(self, src, src_off, dst, dst_off):
        r = self.ROUNDS
        S = self.S
        A = read_u32_le(src, src_off)
        B = read_u32_le(src, src_off + 4)
        C = read_u32_le(src, src_off + 8)
        D = read_u32_le(src, src_off + 12)

        C = u32(C - S[2 * r + 3])
        A = u32(A - S[2 * r + 2])

        for i in range(r, 0, -1):
            # Rotate ABCD right: (A,B,C,D) = (D,A,B,C)
            A, B, C, D = D, A, B, C
            uu = rotl32(mul32(D, u32(2 * D + 1)), 5)
            tt = rotl32(mul32(B, u32(2 * B + 1)), 5)
            C = u32(rotr32(u32(C - S[2 * i + 1]), tt & 31) ^ uu)
            A = u32(rotr32(u32(A - S[2 * i]), uu & 31) ^ tt)

        D = u32(D - S[1])
        B = u32(B - S[0])

        write_u32_le(A, dst, dst_off)
        write_u32_le(B, dst, dst_off + 4)
        write_u32_le(C, dst, dst_off + 8)
        write_u32_le(D, dst, dst_off + 12)

    def block_decrypt(self, in_buf, length, out_buf):
        for i in range(0, length, self.BLOCK_SIZE):
            self.decrypt_block(in_buf, i, out_buf, i)


# ============================================================
#  MARS Cipher (algorithm 5)
#  Block=16, Key=16, Little-endian
# ============================================================

# MARS S-box (512 entries)
MARS_SBOX = [
    0x09d0c479, 0x28c8ffe0, 0x84aa6c39, 0x9dad7287,
    0x7dff9be3, 0xd4268361, 0xc96da1d4, 0x7974cc93,
    0x85d0582e, 0x2a4b5705, 0x1ca16a62, 0xc3bd279d,
    0x0f1f25e5, 0x5160372f, 0xc695c1fb, 0x4d7ff1e4,
    0xae5f6bf4, 0x0d72ee46, 0xff23de8a, 0xb1cf8e83,
    0xf14902e2, 0x3e981e42, 0x8bf53eb6, 0x7f4bf8ac,
    0x83631f83, 0x25970205, 0x76afe784, 0x3a7931d4,
    0x4f846450, 0x5c64c3f6, 0x210a5f18, 0xc6986a26,
    0x28f4e826, 0x3a60a81c, 0xd340a664, 0x7ea820c4,
    0x526687c5, 0x7eddd12b, 0x32a11d1d, 0x9c9ef086,
    0x80f6e831, 0xab6f04ad, 0x56fb9b53, 0x8b2e095c,
    0xb68556ae, 0xd2250b0d, 0x294a7721, 0xe21fb253,
    0xae136749, 0xe82aae86, 0x93365104, 0x99404a66,
    0x78a784dc, 0xb69ba84b, 0x04046793, 0x23db5c1e,
    0x46cae1d6, 0x2fe28134, 0x5a223942, 0x1863cd5b,
    0xc190c6e3, 0x07dfb846, 0x6eb88816, 0x2d0dcc4a,
    0xa4ccae59, 0x3798670d, 0xcbfa9493, 0x4f481d45,
    0xeafc8ca8, 0xdb1129d6, 0xb0449e20, 0x0f5407fb,
    0x6167d9a8, 0xd1f45763, 0x4daa96c3, 0x3bec5958,
    0xababa014, 0xb6ccd201, 0x38d6279f, 0x02682215,
    0x8f376cd5, 0x092c237e, 0xbfc56593, 0x32889d2c,
    0x854b3e95, 0x05bb9b43, 0x7dcd5dcd, 0xa02e926c,
    0xfae527e5, 0x36a1c330, 0x3412e1ae, 0xf257f462,
    0x3c4f1d71, 0x30a2e809, 0x68e5f551, 0x9c61ba44,
    0x5ded0ab8, 0x75ce09c8, 0x9654f93e, 0x698c0cca,
    0x243cb3e4, 0x2b062b97, 0x0f3b8d9e, 0x00e050df,
    0xfc5d6166, 0xe35f9288, 0xc079550d, 0x0591aee8,
    0x8e531e74, 0x75fe3578, 0x2f6d829a, 0xf60b21ae,
    0x95e8eb8d, 0x6699486b, 0x901d7d9b, 0xfd6d6e31,
    0x1090acef, 0xe0670dd8, 0xdab2e692, 0xcd6d4365,
    0xe5393514, 0x3af345f0, 0x6241fc4d, 0x460da3a3,
    0x7bcf3729, 0x8bf1d1e0, 0x14aac070, 0x1587ed55,
    0x3afd7d3e, 0xd2f29e01, 0x29a9d1f6, 0xefb10c53,
    0xcf3b870f, 0xb414935c, 0x664465ed, 0x024acac7,
    0x59a744c1, 0x1d2936a7, 0xdc580aa6, 0xcf574ca8,
    0x040a7a10, 0x6cd81807, 0x8a98be4c, 0xaccea063,
    0xc33e92b5, 0xd1e0e03d, 0xb322517e, 0x2092bd13,
    0x386b2c4a, 0x52e8dd58, 0x58656dfb, 0x50820371,
    0x41811896, 0xe337ef7e, 0xd39fb119, 0xc97f0df6,
    0x68fea01b, 0xa150a6e5, 0x55258962, 0xeb6ff41b,
    0xd7c9cd7a, 0xa619cd9e, 0xbcf09576, 0x2672c073,
    0xf003fb3c, 0x4ab7a50b, 0x1484126a, 0x487ba9b1,
    0xa64fc9c6, 0xf6957d49, 0x38b06a75, 0xdd805fcd,
    0x63d094cf, 0xf51c999e, 0x1aa4d343, 0xb8495294,
    0xce9f8e99, 0xbffcd770, 0xc7c275cc, 0x378453a7,
    0x7b21be33, 0x397f41bd, 0x4e94d131, 0x92cc1f98,
    0x5915ea51, 0x99f861b7, 0xc9980a88, 0x1d74fd5f,
    0xb0a495f8, 0x614deed0, 0xb5778eea, 0x5941792d,
    0xfa90c1f8, 0x33f824b4, 0xc4965372, 0x3ff6d550,
    0x4ca5fec0, 0x8630e964, 0x5b3fbbd6, 0x7da26a48,
    0xb203231a, 0x04297514, 0x2d639306, 0x2eb13149,
    0x16a45272, 0x532459a0, 0x8e5f4872, 0xf966c7d9,
    0x07128dc0, 0x0d44db62, 0xafc8d52d, 0x06316131,
    0xd838e7ce, 0x1bc41d00, 0x3a2e8c0f, 0xea83837e,
    0xb984737d, 0x13ba4891, 0xc4f8b949, 0xa6d6acb3,
    0xa215cdce, 0x8359838b, 0x6bd1aa31, 0xf579dd52,
    0x21b93f93, 0xf5176781, 0x187dfdde, 0xe94aeb76,
    0x2b38fd54, 0x431de1da, 0xab394825, 0x9ad3048f,
    0xdfea32aa, 0x659473e3, 0x623f7863, 0xf3346c59,
    0xab3ab685, 0x3346a90b, 0x6b56443e, 0xc6de01f8,
    0x8d421fc0, 0x9b0ed10c, 0x88f1a1e9, 0x54c1f029,
    0x7dead57b, 0x8d7ba426, 0x4cf5178a, 0x551a7cca,
    0x1a9a5f08, 0xfcd651b9, 0x25605182, 0xe11fc6c3,
    0xb6fd9676, 0x337b3027, 0xb7c8eb14, 0x9e5fd030,
    0x6b57e354, 0xad913cf7, 0x7e16688d, 0x58872a69,
    0x2c2fc7df, 0xe389ccc6, 0x30738df1, 0x0824a734,
    0xe1797a8b, 0xa4a8d57b, 0x5b5d193b, 0xc8a8309b,
    0x73f9a978, 0x73398d32, 0x0f59573e, 0xe9df2b03,
    0xe8a5b6c8, 0x848d0704, 0x98df93c2, 0x720a1dc3,
    0x684f259a, 0x943ba848, 0xa6370152, 0x863b5ea3,
    0xd17b978b, 0x6d9b58ef, 0x0a700dd4, 0xa73d36bf,
    0x8e6a0829, 0x8695bc14, 0xe35b3447, 0x933ac568,
    0x8894b022, 0x2f511c27, 0xddfbcc3c, 0x006662b6,
    0x117c83fe, 0x4e12b414, 0xc2bca766, 0x3a2fec10,
    0xf4562420, 0x55792e2a, 0x46f5d857, 0xceda25ce,
    0xc3601d3b, 0x6c00ab46, 0xefac9c28, 0xb3c35047,
    0x611dfee3, 0x257c3207, 0xfdd58482, 0x3b14d84f,
    0x23becb64, 0xa075f3a3, 0x088f8ead, 0x07adf158,
    0x7796943c, 0xfacabf3d, 0xc09730cd, 0xf7679969,
    0xda44e9ed, 0x2c854c12, 0x35935fa3, 0x2f057d9f,
    0x690624f8, 0x1cb0bafd, 0x7b0dbdc6, 0x810f23bb,
    0xfa929a1a, 0x6d969a17, 0x6742979b, 0x74ac7d05,
    0x010e65c4, 0x86a3d963, 0xf907b5a0, 0xd0042bd3,
    0x158d7d03, 0x287a8255, 0xbba8366f, 0x096edc33,
    0x21916a7b, 0x77b56b86, 0x951622f9, 0xa6c5e650,
    0x8cea17d1, 0xcd8c62bc, 0xa3d63433, 0x358a68fd,
    0x0f9b9d3c, 0xd6aa295b, 0xfe33384a, 0xc000738e,
    0xcd67eb2f, 0xe2eb6dc2, 0x97338b02, 0x06c9f246,
    0x419cf1ad, 0x2b83c045, 0x3723f18a, 0xcb5b3089,
    0x160bead7, 0x5d494656, 0x35f8a74b, 0x1e4e6c9e,
    0x000399bd, 0x67466880, 0xb4174831, 0xacf423b2,
    0xca815ab3, 0x5a6395e7, 0x302a67c5, 0x8bdb446b,
    0x108f8fa4, 0x10223eda, 0x92b8b48b, 0x7f38d0ee,
    0xab2701d4, 0x0262d415, 0xaf224a30, 0xb3d88aba,
    0xf8b2c3af, 0xdaf7ef70, 0xcc97d3b7, 0xe9614b6c,
    0x2baebff4, 0x70f687cf, 0x386c9156, 0xce092ee5,
    0x01e87da6, 0x6ce91e6a, 0xbb7bcc84, 0xc7922c20,
    0x9d3b71fd, 0x060e41c6, 0xd7590f15, 0x4e03bb47,
    0x183c198e, 0x63eeb240, 0x2ddbf49a, 0x6d5cba54,
    0x923750af, 0xf9e14236, 0x7838162b, 0x59726c72,
    0x81b66760, 0xbb2926c1, 0x48a0ce0d, 0xa6c0496d,
    0xad43507b, 0x718d496a, 0x9df057af, 0x44b1bde6,
    0x054356dc, 0xde7ced35, 0xd51a138b, 0x62088cc9,
    0x35830311, 0xc96efca2, 0x686f86ec, 0x8e77cb68,
    0x63e1d6b8, 0xc80f9778, 0x79c491fd, 0x1b4c67f2,
    0x72698d7d, 0x5e368c31, 0xf7d95e2e, 0xa1d3493f,
    0xdcd9433e, 0x896f1552, 0x4bc4ca7a, 0xa6d1baf4,
    0xa5a96dcc, 0x0bef8b46, 0xa169fda7, 0x74df40b7,
    0x4e208804, 0x9a756607, 0x038e87c8, 0x20211e44,
    0x8b7ad4bf, 0xc6403f35, 0x1848e36d, 0x80bdb038,
    0x1e62891c, 0x643d2107, 0xbf04d6f8, 0x21092c8c,
    0xf644f389, 0x0778404e, 0x7b78adb8, 0xa2c52d53,
    0x42157abe, 0xa2253e2e, 0x7bf3f4ae, 0x80f594f9,
    0x953194e7, 0x77eb92ed, 0xb3816930, 0xda8d9336,
    0xbf447469, 0xf26d9483, 0xee6faed5, 0x71371235,
    0xde425f73, 0xb4e59f43, 0x7dbe2d4e, 0x2d37b185,
    0x49dc9a63, 0x98c39d98, 0x1301c9a2, 0x389b1bbf,
    0x0c18588d, 0xa421c1ba, 0x7aa3865c, 0x71e08558,
    0x3c5cfcaa, 0x7d239ca4, 0x0297d9dd, 0xd7dc2830,
    0x4b37802b, 0x7428ab54, 0xaeee0347, 0x4b3fbb85,
    0x692f2f08, 0x134e578e, 0x36d9e0bf, 0xae8b5fcf,
    0xedb93ecf, 0x2b27248e, 0x170eb1ef, 0x7dc57fd6,
    0x1e760f16, 0xb1136601, 0x864e1b9b, 0xd7ea7319,
    0x3ab871bd, 0xcfa4d76f, 0xe31bd782, 0x0dbeb469,
    0xabb96061, 0x5370f85d, 0xffb07e37, 0xda30d0fb,
    0xebc977b6, 0x0b98b40f, 0x3a4d0fe6, 0xdf4fc26b,
    0x159cf22a, 0xc298d6e2, 0x2b78ef6a, 0x61a94ac0,
    0xab561187, 0x14eea0f0, 0xdf0d4164, 0x19af70ee,
]


class MARSCipher:
    BLOCK_SIZE = 16

    def __init__(self, key):
        key = key[:16]
        self.lKey = [0] * 40
        self._set_key(key)

    def get_block_size(self):
        return self.BLOCK_SIZE

    def _set_key(self, in_key):
        S = MARS_SBOX
        T = [0] * 15
        for i in range(len(in_key) // 4):
            T[i] = read_u32_le(in_key, i * 4)
        T[len(in_key) // 4] = u32(len(in_key) // 4)

        for j in range(4):
            for i in range(15):
                T[i] = u32(T[i] ^ rotl32(u32(T[(i + 8) % 15] ^ T[(i + 13) % 15]), 3) ^ u32(4 * i + j))
            for _ in range(4):
                for i in range(15):
                    T[i] = rotl32(u32(T[i] + S[T[(i + 14) % 15] % 512]), 9)
            for i in range(10):
                self.lKey[10 * j + i] = T[(4 * i) % 15]

        for i in range(5, 37, 2):
            w = u32(self.lKey[i] | 3)
            m = _mars_gen_mask(w)
            if m != 0:
                w = u32(w ^ (rotl32(S[265 + (self.lKey[i] & 3)], self.lKey[i - 1] & 31) & m))
            self.lKey[i] = w

    def decrypt_block(self, src, src_off, dst, dst_off):
        K = self.lKey
        S = MARS_SBOX
        d = u32(read_u32_le(src, src_off) + K[36])
        c = u32(read_u32_le(src, src_off + 4) + K[37])
        b = u32(read_u32_le(src, src_off + 8) + K[38])
        a = u32(read_u32_le(src, src_off + 12) + K[39])

        # Forward mixing
        a, b, c, d = _mars_fmix(a, b, c, d, S); a = u32(a + d)
        b, c, d, a = _mars_fmix(b, c, d, a, S); b = u32(b + c)
        c, d, a, b = _mars_fmix(c, d, a, b, S)
        d, a, b, c = _mars_fmix(d, a, b, c, S)
        a, b, c, d = _mars_fmix(a, b, c, d, S); a = u32(a + d)
        b, c, d, a = _mars_fmix(b, c, d, a, S); b = u32(b + c)
        c, d, a, b = _mars_fmix(c, d, a, b, S)
        d, a, b, c = _mars_fmix(d, a, b, c, S)

        # Reverse keyed rounds
        a, b, c, d = _mars_rktr(a, b, c, d, K, S, 34)
        b, c, d, a = _mars_rktr(b, c, d, a, K, S, 32)
        c, d, a, b = _mars_rktr(c, d, a, b, K, S, 30)
        d, a, b, c = _mars_rktr(d, a, b, c, K, S, 28)
        a, b, c, d = _mars_rktr(a, b, c, d, K, S, 26)
        b, c, d, a = _mars_rktr(b, c, d, a, K, S, 24)
        c, d, a, b = _mars_rktr(c, d, a, b, K, S, 22)
        d, a, b, c = _mars_rktr(d, a, b, c, K, S, 20)
        a, d, c, b = _mars_rktr(a, d, c, b, K, S, 18)
        b, a, d, c = _mars_rktr(b, a, d, c, K, S, 16)
        c, b, a, d = _mars_rktr(c, b, a, d, K, S, 14)
        d, c, b, a = _mars_rktr(d, c, b, a, K, S, 12)
        a, d, c, b = _mars_rktr(a, d, c, b, K, S, 10)
        b, a, d, c = _mars_rktr(b, a, d, c, K, S, 8)
        c, b, a, d = _mars_rktr(c, b, a, d, K, S, 6)
        d, c, b, a = _mars_rktr(d, c, b, a, K, S, 4)

        # Backward mixing
        a, b, c, d = _mars_bmix(a, b, c, d, S)
        b, c, d, a = _mars_bmix(b, c, d, a, S); c = u32(c - b)
        c, d, a, b = _mars_bmix(c, d, a, b, S); d = u32(d - a)
        d, a, b, c = _mars_bmix(d, a, b, c, S)
        a, b, c, d = _mars_bmix(a, b, c, d, S)
        b, c, d, a = _mars_bmix(b, c, d, a, S); c = u32(c - b)
        c, d, a, b = _mars_bmix(c, d, a, b, S); d = u32(d - a)
        d, a, b, c = _mars_bmix(d, a, b, c, S)

        write_u32_le(u32(d - K[0]), dst, dst_off)
        write_u32_le(u32(c - K[1]), dst, dst_off + 4)
        write_u32_le(u32(b - K[2]), dst, dst_off + 8)
        write_u32_le(u32(a - K[3]), dst, dst_off + 12)

    def block_decrypt(self, in_buf, length, out_buf):
        for i in range(0, length, self.BLOCK_SIZE):
            self.decrypt_block(in_buf, i, out_buf, i)


def _mars_gen_mask(x):
    m = u32((u32(~x) ^ (x >> 1)) & 0x7FFFFFFF)
    m = u32(m & (m >> 1) & (m >> 2))
    m = u32(m & (m >> 3) & (m >> 6))
    if m == 0:
        return 0
    m = u32(m << 1)
    m = u32(m | (m << 1))
    m = u32(m | (m << 2))
    m = u32(m | (m << 4))
    m = u32(m | ((m << 1) & u32(~x) & 0x80000000))
    return u32(m & 0xFFFFFFFC)

def _mars_fmix(a, b, c, d, S):
    r = rotr32(a, 8)
    b = u32(b ^ S[a & 255])
    b = u32(b + S[(r & 255) + 256])
    r = rotr32(a, 16)
    a = rotr32(a, 24)
    c = u32(c + S[r & 255])
    d = u32(d ^ S[(a & 255) + 256])
    return a, b, c, d

def _mars_bmix(a, b, c, d, S):
    r = rotl32(a, 8)
    b = u32(b ^ S[(a & 255) + 256])
    c = u32(c - S[r & 255])
    r = rotl32(a, 16)
    a = rotl32(a, 24)
    d = u32(d - S[(r & 255) + 256])
    d = u32(d ^ S[a & 255])
    return a, b, c, d

def _mars_rktr(a, b, c, d, K, S, i):
    r = u32(mul32(a, K[i + 1]))
    a = rotr32(a, 13)
    m = u32(a + K[i])
    l0 = S[m & 511]
    r = rotl32(r, 5)
    l = u32(l0 ^ r)
    c = u32(c - rotl32(m, r & 31))
    r = rotl32(r, 5)
    l = u32(l ^ r)
    d = u32(d ^ r)
    b = u32(b - rotl32(l, r & 31))
    return a, b, c, d


# ============================================================
#  IDEA Cipher (algorithm 6)
#  Block=8, Key=16, Big-endian
# ============================================================

class IDEACipher:
    BLOCK_SIZE = 8
    ROUNDS = 8

    def __init__(self, key):
        key = key[:16]
        enc_keys = _idea_expand_key(key)
        self.dec_keys = _idea_invert_keys(enc_keys)

    def get_block_size(self):
        return self.BLOCK_SIZE

    def decrypt_block(self, src, src_off, dst, dst_off):
        K = self.dec_keys
        x0 = read_u16_be(src, src_off)
        x1 = read_u16_be(src, src_off + 2)
        x2 = read_u16_be(src, src_off + 4)
        x3 = read_u16_be(src, src_off + 6)

        p = 0
        for _ in range(self.ROUNDS):
            x0 = _idea_mul_mod(x0, K[p]); p += 1
            x1 = _idea_add_mod(x1, K[p]); p += 1
            x2 = _idea_add_mod(x2, K[p]); p += 1
            x3 = _idea_mul_mod(x3, K[p]); p += 1

            t0 = x1
            t1 = x2
            x2 = x2 ^ x0
            x1 = x1 ^ x3
            x2 = _idea_mul_mod(x2, K[p]); p += 1
            x1 = _idea_add_mod(x1, x2)
            x1 = _idea_mul_mod(x1, K[p]); p += 1
            x2 = _idea_add_mod(x2, x1)

            x0 = x0 ^ x1
            x3 = x3 ^ x2
            x1 = x1 ^ t1
            x2 = x2 ^ t0

        # Output transform
        X1 = _idea_mul_mod(x0, K[p]); p += 1
        X2 = _idea_add_mod(x2, K[p]); p += 1
        X3 = _idea_add_mod(x1, K[p]); p += 1
        X4 = _idea_mul_mod(x3, K[p]); p += 1

        write_u16_be(X1, dst, dst_off)
        write_u16_be(X2, dst, dst_off + 2)
        write_u16_be(X3, dst, dst_off + 4)
        write_u16_be(X4, dst, dst_off + 6)

    def block_decrypt(self, in_buf, length, out_buf):
        for i in range(0, length, self.BLOCK_SIZE):
            self.decrypt_block(in_buf, i, out_buf, i)


def _idea_expand_key(key):
    Z = [0] * 52
    for i in range(8):
        Z[i] = read_u16_be(key, i * 2)
    for i in range(8, 52):
        mod = i & 7
        if mod == 6:
            Z[i] = ((Z[i - 7] << 9) | (Z[i - 14] >> 7)) & 0xFFFF
        elif mod == 7:
            Z[i] = ((Z[i - 15] << 9) | (Z[i - 14] >> 7)) & 0xFFFF
        else:
            Z[i] = ((Z[i - 7] << 9) | (Z[i - 6] >> 7)) & 0xFFFF
    return Z

def _idea_invert_keys(enc):
    dec = [0] * 52
    p = 0
    q = 52

    t1 = _idea_mul_inverse(enc[p]); p += 1
    t2 = _idea_add_inverse(enc[p]); p += 1
    t3 = _idea_add_inverse(enc[p]); p += 1
    t4 = _idea_mul_inverse(enc[p]); p += 1
    q -= 1; dec[q] = t4
    q -= 1; dec[q] = t3
    q -= 1; dec[q] = t2
    q -= 1; dec[q] = t1

    for r in range(1, 8):
        t1 = enc[p]; p += 1
        t2 = enc[p]; p += 1
        q -= 1; dec[q] = t2
        q -= 1; dec[q] = t1

        t1 = _idea_mul_inverse(enc[p]); p += 1
        t2 = _idea_add_inverse(enc[p]); p += 1
        t3 = _idea_add_inverse(enc[p]); p += 1
        t4 = _idea_mul_inverse(enc[p]); p += 1
        q -= 1; dec[q] = t4
        q -= 1; dec[q] = t2
        q -= 1; dec[q] = t3
        q -= 1; dec[q] = t1

    t1 = enc[p]; p += 1
    t2 = enc[p]; p += 1
    q -= 1; dec[q] = t2
    q -= 1; dec[q] = t1

    t1 = _idea_mul_inverse(enc[p]); p += 1
    t2 = _idea_add_inverse(enc[p]); p += 1
    t3 = _idea_add_inverse(enc[p]); p += 1
    t4 = _idea_mul_inverse(enc[p]); p += 1
    q -= 1; dec[q] = t4
    q -= 1; dec[q] = t3
    q -= 1; dec[q] = t2
    q -= 1; dec[q] = t1

    return dec

def _idea_mul_mod(a, b):
    a = a & 0xFFFF
    b = b & 0xFFFF
    if a == 0: a = 0x10000
    if b == 0: b = 0x10000
    r = (a * b) % 0x10001
    return 0 if r == 0x10000 else r & 0xFFFF

def _idea_add_mod(a, b):
    return (a + b) & 0xFFFF

def _idea_mul_inverse(x):
    x = x & 0xFFFF
    if x <= 1:
        return x
    t1 = 0x10001 // x
    y = 0x10001 % x
    if y == 1:
        return (0x10001 - t1) & 0xFFFF
    t0 = 1
    while y != 1:
        q = x // y
        x = x % y
        t0 = (t0 + q * t1) % 0x10001
        if x == 1:
            return t0 & 0xFFFF
        q2 = y // x
        y = y % x
        t1 = (t1 + q2 * t0) % 0x10001
    return (0x10001 - t1) & 0xFFFF

def _idea_add_inverse(x):
    return (0x10000 - (x & 0xFFFF)) & 0xFFFF


# ============================================================
#  GOST 28147-89 Cipher (algorithm 7)
#  Block=8, Key=32, Little-endian
# ============================================================

GOST_SBOX = [
    [ 4, 10,  9,  2, 13,  8,  0, 14,  6, 11,  1, 12,  7, 15,  5,  3],
    [14, 11,  4, 12,  6, 13, 15, 10,  2,  3,  8,  1,  0,  7,  5,  9],
    [ 5,  8,  1, 13, 10,  3,  4,  2, 14, 15, 12,  7,  6,  0,  9, 11],
    [ 7, 13, 10,  1,  0,  8,  9, 15, 14,  4,  6, 12, 11,  2,  5,  3],
    [ 6, 12,  7,  1,  5, 15, 13,  8,  4, 10,  9, 14,  0,  3, 11,  2],
    [ 4, 11, 10,  0,  7,  2,  1, 13,  3,  6,  8,  5,  9, 12, 15, 14],
    [13, 11,  4,  1,  3, 15,  5,  9,  0, 10, 14,  7,  6,  8,  2, 12],
    [ 1, 15, 13,  0,  5,  7, 10,  4,  9,  2,  3, 14,  6, 11,  8, 12],
]

# Pre-compute lookup tables
def _gost_build_lookup():
    tables = []
    for k in range(4):
        table = [0] * 256
        s_low = GOST_SBOX[2 * k]
        s_high = GOST_SBOX[2 * k + 1]
        for i in range(256):
            lo = s_low[i & 0x0F]
            hi = s_high[(i >> 4) & 0x0F]
            table[i] = (lo | (hi << 4)) << (8 * k)
        tables.append(table)
    return tables

GOST_LOOKUP = _gost_build_lookup()

def _gost_sbox_substitute(value):
    value = u32(value)
    return u32(
        GOST_LOOKUP[0][value & 0xFF] |
        GOST_LOOKUP[1][(value >> 8) & 0xFF] |
        GOST_LOOKUP[2][(value >> 16) & 0xFF] |
        GOST_LOOKUP[3][(value >> 24) & 0xFF]
    )

def _gost_rotl11(x):
    return u32((x << 11) | (u32(x) >> 21))


class GOSTCipher:
    BLOCK_SIZE = 8
    ROUNDS = 32

    def __init__(self, key):
        key = key[:32]
        self.K = [read_u32_le(key, i * 4) for i in range(8)]
        # Pre-compute decryption schedule
        self.dec_schedule = [0] * 32
        for i in range(8):
            self.dec_schedule[i] = self.K[i]
        for rep in range(3):
            for i in range(8):
                self.dec_schedule[8 + rep * 8 + i] = self.K[7 - i]

    def get_block_size(self):
        return self.BLOCK_SIZE

    def decrypt_block(self, src, src_off, dst, dst_off):
        n1 = read_u32_le(src, src_off)
        n2 = read_u32_le(src, src_off + 4)
        ks = self.dec_schedule

        for i in range(31):
            temp = u32(n1 + ks[i])
            substituted = _gost_sbox_substitute(temp)
            rotated = _gost_rotl11(substituted)
            new_n1 = u32(n2 ^ rotated)
            n2 = n1
            n1 = new_n1

        # Last round: no swap
        temp = u32(n1 + ks[31])
        substituted = _gost_sbox_substitute(temp)
        rotated = _gost_rotl11(substituted)
        n2 = u32(n2 ^ rotated)

        write_u32_le(n1, dst, dst_off)
        write_u32_le(n2, dst, dst_off + 4)

    def block_decrypt(self, in_buf, length, out_buf):
        for i in range(0, length, self.BLOCK_SIZE):
            self.decrypt_block(in_buf, i, out_buf, i)


# ============================================================
#  CAST5 Cipher (algorithm 2)
#  Block=8, Key=16, Big-endian
#  (Minimal stub - uses pycryptodome for CAST5)
# ============================================================

class CAST5Cipher:
    """CAST5 using pycryptodome for the heavy lifting."""
    BLOCK_SIZE = 8

    def __init__(self, key):
        from Crypto.Cipher import CAST
        key = bytes(key[:16])
        self._cipher = CAST.new(key, CAST.MODE_ECB)

    def get_block_size(self):
        return self.BLOCK_SIZE

    def block_decrypt(self, in_buf, length, out_buf):
        # Process block by block using ECB
        for i in range(0, length, self.BLOCK_SIZE):
            block = bytes(in_buf[i:i + self.BLOCK_SIZE])
            dec = self._cipher.decrypt(block)
            out_buf[i:i + self.BLOCK_SIZE] = dec


# ============================================================
#  Cipher factory
# ============================================================

CIPHER_NAMES = ['TEA', 'ThreeWay', 'CAST5', 'RC5', 'RC6', 'MARS', 'IDEA', 'GOST']

def init_cipher(algorithm, key):
    algo = algorithm & 7
    key = bytes(key)
    if algo == 0:
        return TEACipher(key)
    elif algo == 1:
        return ThreeWayCipher(key)
    elif algo == 2:
        return CAST5Cipher(key)
    elif algo == 3:
        return RC5Cipher(key)
    elif algo == 4:
        return RC6Cipher(key)
    elif algo == 5:
        return MARSCipher(key)
    elif algo == 6:
        return IDEACipher(key)
    elif algo == 7:
        return GOSTCipher(key)
    else:
        raise ValueError(f"Unknown algorithm: {algo}")


# ============================================================
#  ModulusCryptor
# ============================================================

KEY_1 = b'webzen#@!01webzen#@!01webzen#@!0'  # 32 bytes

def decrypt_modulus_cryptor(source):
    """
    Decrypt data encrypted with Webzen's ModulusCryptor.
    Ported from the TypeScript implementation in modulus-cryptor.ts.
    """
    if len(source) < 34:
        raise ValueError("ModulusCryptor: source buffer too short")

    buf = bytearray(source)  # clone
    size = len(buf)
    data_size = size - 34

    # NOTE: algo assignment is swapped vs naive C# reading
    # In TS: algorithm1 = buf[1], algorithm2 = buf[0]
    algorithm1 = buf[1]
    algorithm2 = buf[0]

    print(f"  [ModulusCryptor] size={size}, dataSize={data_size}, "
          f"algo1={algorithm1 & 7}({CIPHER_NAMES[algorithm1 & 7]}), "
          f"algo2={algorithm2 & 7}({CIPHER_NAMES[algorithm2 & 7]})")

    # Stage 1: partial decrypt to recover key_2
    cipher1 = init_cipher(algorithm1, KEY_1)
    block_size = 1024 - (1024 % cipher1.get_block_size())
    print(f"  [ModulusCryptor] Stage1: cipher={CIPHER_NAMES[algorithm1 & 7]}, "
          f"blockSize={block_size}, cipherBlockSize={cipher1.get_block_size()}")

    if data_size > 4 * block_size:
        index = 2 + (data_size >> 1)
        print(f"  [ModulusCryptor] Stage1: middle block at index={index}, len={block_size}")
        block = bytearray(buf[index:index + block_size])
        out_block = bytearray(block_size)
        cipher1.block_decrypt(block, len(block), out_block)
        buf[index:index + block_size] = out_block

    if data_size > block_size:
        # End block
        index = size - block_size
        print(f"  [ModulusCryptor] Stage1: end block at index={index}, len={block_size}")
        block = bytearray(buf[index:index + block_size])
        out_block = bytearray(block_size)
        cipher1.block_decrypt(block, len(block), out_block)
        buf[index:index + block_size] = out_block

        # Start block
        index = 2
        print(f"  [ModulusCryptor] Stage1: start block at index={index}, len={block_size}")
        block = bytearray(buf[index:index + block_size])
        out_block = bytearray(block_size)
        cipher1.block_decrypt(block, len(block), out_block)
        buf[index:index + block_size] = out_block

    # Extract key_2 (bytes 2..34)
    key2 = bytes(buf[2:34])
    print(f"  [ModulusCryptor] key2 (first 16): [{' '.join(f'{b:02x}' for b in key2[:16])}]")

    # Stage 2: decrypt actual data using key_2
    cipher2 = init_cipher(algorithm2, key2)
    decrypt_size = data_size - (data_size % cipher2.get_block_size())
    print(f"  [ModulusCryptor] Stage2: cipher={CIPHER_NAMES[algorithm2 & 7]}, "
          f"decryptSize={decrypt_size}, cipherBlockSize={cipher2.get_block_size()}")

    if decrypt_size > 0:
        data_start = 34
        block = bytearray(buf[data_start:data_start + decrypt_size])
        out_block = bytearray(decrypt_size)
        cipher2.block_decrypt(block, len(block), out_block)
        buf[data_start:data_start + decrypt_size] = out_block

    result = bytes(buf[34:])
    print(f"  [ModulusCryptor] Result first 10: [{', '.join(str(b) for b in result[:10])}]")
    return result


# ============================================================
#  FileCryptor & BuxCryptor
# ============================================================

MAP_XOR_KEY = bytes([
    0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
    0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2,
])

def decrypt_file_cryptor(src):
    """FileCryptor XOR decryption."""
    dst = bytearray(len(src))
    map_key = 0x5E
    for i in range(len(src)):
        dst[i] = ((src[i] ^ MAP_XOR_KEY[i & 15]) - map_key) & 0xFF
        map_key = (src[i] + 0x3D) & 0xFF
    return bytes(dst)


BUX_MASK = bytes([0xFC, 0xCF, 0xAB])

def xor_bux_mask(buf):
    """BuxCryptor XOR mask."""
    out = bytearray(len(buf))
    for i in range(len(buf)):
        out[i] = buf[i] ^ BUX_MASK[i % 3]
    return bytes(out)


# ============================================================
#  LEA-256 ECB Decrypt
# ============================================================

LEA_KEY_DELTA = [
    0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec,
    0x715ea49e, 0xc785da0a, 0xe04ef22a, 0xe5c40957
]

def _lea_key_schedule_256(key_words):
    rk = [0] * 192
    T = list(key_words)
    for i in range(32):
        d = LEA_KEY_DELTA[i & 7]
        s = (i * 6) & 7
        T[(s+0)&7] = rotl32(u32(T[(s+0)&7] + rotl32(d, i    )), 1)
        T[(s+1)&7] = rotl32(u32(T[(s+1)&7] + rotl32(d, i + 1)), 3)
        T[(s+2)&7] = rotl32(u32(T[(s+2)&7] + rotl32(d, i + 2)), 6)
        T[(s+3)&7] = rotl32(u32(T[(s+3)&7] + rotl32(d, i + 3)), 11)
        T[(s+4)&7] = rotl32(u32(T[(s+4)&7] + rotl32(d, i + 4)), 13)
        T[(s+5)&7] = rotl32(u32(T[(s+5)&7] + rotl32(d, i + 5)), 17)
        for j in range(6):
            rk[i * 6 + j] = T[(s + j) & 7]
    return rk

def _lea_round_dec(s, rk6):
    t = [0] * 4
    t[0] = s[3]
    t[1] = u32(rotr32(s[0], 9) - u32(t[0] ^ rk6[0]) ^ rk6[1])
    t[2] = u32(rotl32(s[1], 5) - u32(t[1] ^ rk6[2]) ^ rk6[3])
    t[3] = u32(rotl32(s[2], 3) - u32(t[2] ^ rk6[4]) ^ rk6[5])
    return t

def lea_256_ecb_decrypt(key, data):
    """Decrypt data with LEA-256 in ECB mode."""
    if len(key) != 32:
        raise ValueError("LEA-256 key must be 32 bytes")
    if len(data) % 16 != 0:
        raise ValueError("LEA-ECB data must be multiple of 16 bytes")

    key_words = [read_u32_le(key, i * 4) for i in range(8)]
    RK = _lea_key_schedule_256(key_words)

    out = bytearray(data)
    for off in range(0, len(out), 16):
        state = [read_u32_le(out, off + i * 4) for i in range(4)]
        for r in range(32):
            rk6 = RK[(31 - r) * 6:(32 - r) * 6]
            state = _lea_round_dec(state, rk6)
        for i in range(4):
            write_u32_le(state[i], out, off + i * 4)
    return bytes(out)


# ============================================================
#  Validation
# ============================================================

OBJ_BASE_SIZES = {0: 32, 1: 34, 2: 35, 3: 47, 4: 48, 5: 56}

def validate_obj_structure(data, label=""):
    """Check if decrypted data looks like a valid OBJ structure."""
    if len(data) < 4:
        return False, "Too short"

    version = data[0]
    map_number = data[1]
    count = struct.unpack_from('<h', data, 2)[0]

    if version > 5:
        return False, f"Invalid version {version}"

    obj_size = OBJ_BASE_SIZES.get(version)
    if obj_size is None:
        return False, f"Unknown version {version}"

    expected_size = 4 + count * obj_size
    remaining = len(data) - 4

    if count < 0:
        return False, f"Negative count {count}"
    if count > 10000:
        return False, f"Count too large: {count}"

    match = (remaining == count * obj_size)

    info = (f"version={version}, mapNumber={map_number}, count={count}, "
            f"objSize={obj_size}, expected_data={count * obj_size}, "
            f"actual_remaining={remaining}, match={match}")

    if match and count > 0:
        # Try to read first object and check if float values are reasonable
        off = 4
        obj_type = struct.unpack_from('<h', data, off)[0]
        px, py, pz = struct.unpack_from('<fff', data, off + 2)
        ax, ay, az = struct.unpack_from('<fff', data, off + 14)
        scale = struct.unpack_from('<f', data, off + 26)[0]
        info += (f"\n    First object: type={obj_type}, "
                 f"pos=({px:.1f}, {py:.1f}, {pz:.1f}), "
                 f"angle=({ax:.1f}, {ay:.1f}, {az:.1f}), "
                 f"scale={scale:.3f}")
        # Sanity: positions should be in a reasonable range for MU maps
        if -100000 < px < 100000 and -100000 < py < 100000 and -100000 < pz < 100000:
            if 0 < scale < 100:
                return True, info
            else:
                return True, info + " [WARNING: unusual scale]"
        else:
            return False, info + " [FAIL: positions out of range]"

    return match, info


def print_hex(data, n=40):
    """Print first n bytes as hex."""
    hex_str = ' '.join(f'{b:02x}' for b in data[:n])
    print(f"    Hex[0:{n}]: {hex_str}")


# ============================================================
#  Main test
# ============================================================

def main():
    file_path = "/home/allanbatista/Workspaces/MuData/Season20/World1/EncTerrain1.obj"
    if not os.path.exists(file_path):
        print(f"ERROR: File not found: {file_path}")
        sys.exit(1)

    with open(file_path, 'rb') as f:
        raw = f.read()

    print(f"File: {file_path}")
    print(f"Size: {len(raw)} bytes")
    print(f"First 40 bytes: {' '.join(f'{b:02x}' for b in raw[:40])}")
    print()

    # ---- Test 1: ModulusCryptor on entire file ----
    print("=" * 80)
    print("TEST 1: ModulusCryptor on ENTIRE file")
    print("=" * 80)
    try:
        result = decrypt_modulus_cryptor(raw)
        print_hex(result)
        valid, info = validate_obj_structure(result, "ModulusCryptor(full)")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 2: ModulusCryptor on file[4:] (strip 4-byte header) ----
    print("=" * 80)
    print("TEST 2: ModulusCryptor on file[4:] (strip 4-byte header)")
    print("=" * 80)
    try:
        result4 = decrypt_modulus_cryptor(raw[4:])
        print_hex(result4)
        valid, info = validate_obj_structure(result4, "ModulusCryptor(strip4)")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 3: FileCryptor alone ----
    print("=" * 80)
    print("TEST 3: FileCryptor alone (on entire file)")
    print("=" * 80)
    try:
        fc_result = decrypt_file_cryptor(raw)
        print_hex(fc_result)
        valid, info = validate_obj_structure(fc_result, "FileCryptor")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 4: ModulusCryptor then FileCryptor ----
    print("=" * 80)
    print("TEST 4: ModulusCryptor then FileCryptor")
    print("=" * 80)
    try:
        mc_result = decrypt_modulus_cryptor(raw)
        mcfc_result = decrypt_file_cryptor(mc_result)
        print_hex(mcfc_result)
        valid, info = validate_obj_structure(mcfc_result, "MC+FC")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 5: BuxCryptor alone ----
    print("=" * 80)
    print("TEST 5: BuxCryptor alone (XOR mask)")
    print("=" * 80)
    try:
        bux_result = xor_bux_mask(raw)
        print_hex(bux_result)
        valid, info = validate_obj_structure(bux_result, "BuxCryptor")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 6: BuxCryptor then FileCryptor ----
    print("=" * 80)
    print("TEST 6: BuxCryptor then FileCryptor")
    print("=" * 80)
    try:
        bux_fc = decrypt_file_cryptor(xor_bux_mask(raw))
        print_hex(bux_fc)
        valid, info = validate_obj_structure(bux_fc, "Bux+FC")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 7: FileCryptor then BuxCryptor ----
    print("=" * 80)
    print("TEST 7: FileCryptor then BuxCryptor")
    print("=" * 80)
    try:
        fc_bux = xor_bux_mask(decrypt_file_cryptor(raw))
        print_hex(fc_bux)
        valid, info = validate_obj_structure(fc_bux, "FC+Bux")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 8: ModulusCryptor with swapped algo assignment ----
    # The TS code uses algo1=buf[1], algo2=buf[0]
    # Let's also try the naive C# way: algo1=buf[0], algo2=buf[1]
    print("=" * 80)
    print("TEST 8: ModulusCryptor with NAIVE algo assignment (algo1=buf[0], algo2=buf[1])")
    print("=" * 80)
    try:
        result_naive = decrypt_modulus_cryptor_naive(raw)
        print_hex(result_naive)
        valid, info = validate_obj_structure(result_naive, "MC-naive")
        print(f"  Valid OBJ: {valid}")
        print(f"  Info: {info}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 9: ModulusCryptor (full) then check if result needs FileCryptor ----
    print("=" * 80)
    print("TEST 9: ModulusCryptor then check multiple post-processing")
    print("=" * 80)
    try:
        mc_raw = decrypt_modulus_cryptor(raw)
        # a) Check raw result
        valid_raw, info_raw = validate_obj_structure(mc_raw)
        print(f"  Raw MC result valid: {valid_raw} - {info_raw}")
        # b) Try FileCryptor on result
        fc_mc = decrypt_file_cryptor(mc_raw)
        valid_fc, info_fc = validate_obj_structure(fc_mc)
        print(f"  MC+FileCryptor valid: {valid_fc} - {info_fc}")
        # c) Try BuxCryptor on result
        bux_mc = xor_bux_mask(mc_raw)
        valid_bux, info_bux = validate_obj_structure(bux_mc)
        print(f"  MC+BuxCryptor valid: {valid_bux} - {info_bux}")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 10: LEA-256 on full file (16-byte aligned) ----
    print("=" * 80)
    print("TEST 10: LEA-256 ECB on entire file (key = KEY_1)")
    print("=" * 80)
    try:
        # File is 84994 bytes, not 16-byte aligned. Decrypt aligned portion.
        aligned_len = len(raw) - (len(raw) % 16)
        if aligned_len > 0:
            lea_result = lea_256_ecb_decrypt(KEY_1, raw[:aligned_len])
            print_hex(lea_result)
            valid, info = validate_obj_structure(lea_result)
            print(f"  Valid OBJ: {valid}")
            print(f"  Info: {info}")
        else:
            print("  File not 16-byte aligned")
    except Exception as e:
        print(f"  ERROR: {e}")
    print()

    # ---- Test 11: Try on multiple files to see if pattern holds ----
    print("=" * 80)
    print("TEST 11: ModulusCryptor on multiple Season20 OBJ files")
    print("=" * 80)
    base_dir = "/home/allanbatista/Workspaces/MuData/Season20/"
    test_files = []
    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f.endswith('.obj'):
                test_files.append(os.path.join(root, f))
    test_files.sort()

    success_count = 0
    fail_count = 0
    for fpath in test_files[:10]:  # Test first 10
        fname = os.path.basename(fpath)
        try:
            with open(fpath, 'rb') as f:
                fdata = f.read()
            result = decrypt_modulus_cryptor(fdata)
            valid, info = validate_obj_structure(result)
            status = "OK" if valid else "FAIL"
            if valid:
                success_count += 1
            else:
                fail_count += 1
            print(f"  {fname}: {status} - {info}")
        except Exception as e:
            fail_count += 1
            print(f"  {fname}: ERROR - {e}")

    print(f"\n  Summary: {success_count} OK, {fail_count} FAIL out of {min(len(test_files), 10)} files")
    print()


def decrypt_modulus_cryptor_naive(source):
    """
    Alternative: algo1=buf[0] & 7, algo2=buf[1] & 7  (C# style, not TS style).
    """
    if len(source) < 34:
        raise ValueError("ModulusCryptor: source buffer too short")

    buf = bytearray(source)
    size = len(buf)
    data_size = size - 34

    # Naive C# assignment
    algorithm1 = buf[0]
    algorithm2 = buf[1]

    print(f"  [MC-naive] size={size}, dataSize={data_size}, "
          f"algo1={algorithm1 & 7}({CIPHER_NAMES[algorithm1 & 7]}), "
          f"algo2={algorithm2 & 7}({CIPHER_NAMES[algorithm2 & 7]})")

    cipher1 = init_cipher(algorithm1, KEY_1)
    block_size = 1024 - (1024 % cipher1.get_block_size())

    if data_size > 4 * block_size:
        index = 2 + (data_size >> 1)
        block = bytearray(buf[index:index + block_size])
        out_block = bytearray(block_size)
        cipher1.block_decrypt(block, len(block), out_block)
        buf[index:index + block_size] = out_block

    if data_size > block_size:
        index = size - block_size
        block = bytearray(buf[index:index + block_size])
        out_block = bytearray(block_size)
        cipher1.block_decrypt(block, len(block), out_block)
        buf[index:index + block_size] = out_block

        index = 2
        block = bytearray(buf[index:index + block_size])
        out_block = bytearray(block_size)
        cipher1.block_decrypt(block, len(block), out_block)
        buf[index:index + block_size] = out_block

    key2 = bytes(buf[2:34])
    print(f"  [MC-naive] key2 (first 16): [{' '.join(f'{b:02x}' for b in key2[:16])}]")

    cipher2 = init_cipher(algorithm2, key2)
    decrypt_size = data_size - (data_size % cipher2.get_block_size())

    if decrypt_size > 0:
        data_start = 34
        block = bytearray(buf[data_start:data_start + decrypt_size])
        out_block = bytearray(decrypt_size)
        cipher2.block_decrypt(block, len(block), out_block)
        buf[data_start:data_start + decrypt_size] = out_block

    return bytes(buf[34:])


if __name__ == '__main__':
    main()
