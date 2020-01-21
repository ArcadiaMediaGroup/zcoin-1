/* Copyright (c) 2012-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "core/or/or.h"
#include "test/test.h"

#define RENDSERVICE_PRIVATE
#include "feature/rend/rendservice.h"

static uint8_t v0_test_plaintext[] =
    /* 20 bytes of rendezvous point nickname */
  { 0x4e, 0x69, 0x63, 0x6b, 0x6e, 0x61, 0x6d, 0x65,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    /* 20 bytes dummy rendezvous cookie */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 128 bytes dummy DH handshake data */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

static uint8_t v1_test_plaintext[] =
    /* Version byte */
  { 0x01,
    /* 42 bytes of dummy rendezvous point hex digest */
    0x24, 0x30, 0x30, 0x30, 0x31, 0x30, 0x32, 0x30,
    0x33, 0x30, 0x34, 0x30, 0x35, 0x30, 0x36, 0x30,
    0x37, 0x30, 0x38, 0x30, 0x39, 0x30, 0x41, 0x30,
    0x42, 0x30, 0x43, 0x30, 0x44, 0x30, 0x45, 0x30,
    0x46, 0x31, 0x30, 0x31, 0x31, 0x31, 0x32, 0x31,
    0x33, 0x00,
    /* 20 bytes dummy rendezvous cookie */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 128 bytes dummy DH handshake data */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

static uint8_t v2_test_plaintext[] =
    /* Version byte */
  { 0x02,
    /* 4 bytes rendezvous point's IP address */
    0xc0, 0xa8, 0x00, 0x01,
    /* 2 bytes rendezvous point's OR port */
    0x23, 0x5a,
    /* 20 bytes dummy rendezvous point's identity digest */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 2 bytes length of onion key */
    0x00, 0x8c,
    /* Onion key (140 bytes taken from live test) */
    0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xb1,
    0xcd, 0x46, 0xa9, 0x18, 0xd2, 0x0f, 0x01, 0xf8,
    0xb2, 0xad, 0xa4, 0x79, 0xb4, 0xbb, 0x4b, 0xf4,
    0x54, 0x1e, 0x3f, 0x03, 0x54, 0xcf, 0x7c, 0xb6,
    0xb5, 0xf0, 0xfe, 0xed, 0x4b, 0x7d, 0xd7, 0x61,
    0xdb, 0x6d, 0xd9, 0x19, 0xe2, 0x72, 0x04, 0xaa,
    0x3e, 0x89, 0x26, 0x14, 0x62, 0x9a, 0x6c, 0x11,
    0x0b, 0x35, 0x99, 0x2c, 0x9f, 0x2c, 0x64, 0xa1,
    0xd9, 0xe2, 0x88, 0xce, 0xf6, 0x54, 0xfe, 0x1d,
    0x37, 0x5e, 0x6d, 0x73, 0x95, 0x54, 0x90, 0xf0,
    0x7b, 0xfa, 0xd4, 0x44, 0xac, 0xb2, 0x23, 0x9f,
    0x75, 0x36, 0xe2, 0x78, 0x62, 0x82, 0x80, 0xa4,
    0x23, 0x22, 0xc9, 0xbf, 0xc4, 0x36, 0xd1, 0x31,
    0x33, 0x8e, 0x64, 0xb4, 0xa9, 0x74, 0xa1, 0xcb,
    0x42, 0x8d, 0x60, 0xc7, 0xbb, 0x8e, 0x6e, 0x0f,
    0x36, 0x74, 0x8e, 0xf4, 0x08, 0x99, 0x06, 0x92,
    0xb1, 0x3f, 0xb3, 0xdd, 0xed, 0xf7, 0xc9, 0x02,
    0x03, 0x01, 0x00, 0x01,
    /* 20 bytes dummy rendezvous cookie */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 128 bytes dummy DH handshake data */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

static uint8_t v3_no_auth_test_plaintext[] =
    /* Version byte */
  { 0x03,
    /* Auth type (0 for no auth len/auth data) */
    0x00,
    /* Timestamp */
    0x50, 0x0b, 0xb5, 0xaa,
    /* 4 bytes rendezvous point's IP address */
    0xc0, 0xa8, 0x00, 0x01,
    /* 2 bytes rendezvous point's OR port */
    0x23, 0x5a,
    /* 20 bytes dummy rendezvous point's identity digest */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 2 bytes length of onion key */
    0x00, 0x8c,
    /* Onion key (140 bytes taken from live test) */
    0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xb1,
    0xcd, 0x46, 0xa9, 0x18, 0xd2, 0x0f, 0x01, 0xf8,
    0xb2, 0xad, 0xa4, 0x79, 0xb4, 0xbb, 0x4b, 0xf4,
    0x54, 0x1e, 0x3f, 0x03, 0x54, 0xcf, 0x7c, 0xb6,
    0xb5, 0xf0, 0xfe, 0xed, 0x4b, 0x7d, 0xd7, 0x61,
    0xdb, 0x6d, 0xd9, 0x19, 0xe2, 0x72, 0x04, 0xaa,
    0x3e, 0x89, 0x26, 0x14, 0x62, 0x9a, 0x6c, 0x11,
    0x0b, 0x35, 0x99, 0x2c, 0x9f, 0x2c, 0x64, 0xa1,
    0xd9, 0xe2, 0x88, 0xce, 0xf6, 0x54, 0xfe, 0x1d,
    0x37, 0x5e, 0x6d, 0x73, 0x95, 0x54, 0x90, 0xf0,
    0x7b, 0xfa, 0xd4, 0x44, 0xac, 0xb2, 0x23, 0x9f,
    0x75, 0x36, 0xe2, 0x78, 0x62, 0x82, 0x80, 0xa4,
    0x23, 0x22, 0xc9, 0xbf, 0xc4, 0x36, 0xd1, 0x31,
    0x33, 0x8e, 0x64, 0xb4, 0xa9, 0x74, 0xa1, 0xcb,
    0x42, 0x8d, 0x60, 0xc7, 0xbb, 0x8e, 0x6e, 0x0f,
    0x36, 0x74, 0x8e, 0xf4, 0x08, 0x99, 0x06, 0x92,
    0xb1, 0x3f, 0xb3, 0xdd, 0xed, 0xf7, 0xc9, 0x02,
    0x03, 0x01, 0x00, 0x01,
    /* 20 bytes dummy rendezvous cookie */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 128 bytes dummy DH handshake data */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

static uint8_t v3_basic_auth_test_plaintext[] =
    /* Version byte */
  { 0x03,
    /* Auth type (1 for REND_BASIC_AUTH) */
    0x01,
    /* Auth len (must be 16 bytes for REND_BASIC_AUTH) */
    0x00, 0x10,
    /* Auth data (a 16-byte dummy descriptor cookie) */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    /* Timestamp */
    0x50, 0x0b, 0xb5, 0xaa,
    /* 4 bytes rendezvous point's IP address */
    0xc0, 0xa8, 0x00, 0x01,
    /* 2 bytes rendezvous point's OR port */
    0x23, 0x5a,
    /* 20 bytes dummy rendezvous point's identity digest */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 2 bytes length of onion key */
    0x00, 0x8c,
    /* Onion key (140 bytes taken from live test) */
    0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xb1,
    0xcd, 0x46, 0xa9, 0x18, 0xd2, 0x0f, 0x01, 0xf8,
    0xb2, 0xad, 0xa4, 0x79, 0xb4, 0xbb, 0x4b, 0xf4,
    0x54, 0x1e, 0x3f, 0x03, 0x54, 0xcf, 0x7c, 0xb6,
    0xb5, 0xf0, 0xfe, 0xed, 0x4b, 0x7d, 0xd7, 0x61,
    0xdb, 0x6d, 0xd9, 0x19, 0xe2, 0x72, 0x04, 0xaa,
    0x3e, 0x89, 0x26, 0x14, 0x62, 0x9a, 0x6c, 0x11,
    0x0b, 0x35, 0x99, 0x2c, 0x9f, 0x2c, 0x64, 0xa1,
    0xd9, 0xe2, 0x88, 0xce, 0xf6, 0x54, 0xfe, 0x1d,
    0x37, 0x5e, 0x6d, 0x73, 0x95, 0x54, 0x90, 0xf0,
    0x7b, 0xfa, 0xd4, 0x44, 0xac, 0xb2, 0x23, 0x9f,
    0x75, 0x36, 0xe2, 0x78, 0x62, 0x82, 0x80, 0xa4,
    0x23, 0x22, 0xc9, 0xbf, 0xc4, 0x36, 0xd1, 0x31,
    0x33, 0x8e, 0x64, 0xb4, 0xa9, 0x74, 0xa1, 0xcb,
    0x42, 0x8d, 0x60, 0xc7, 0xbb, 0x8e, 0x6e, 0x0f,
    0x36, 0x74, 0x8e, 0xf4, 0x08, 0x99, 0x06, 0x92,
    0xb1, 0x3f, 0xb3, 0xdd, 0xed, 0xf7, 0xc9, 0x02,
    0x03, 0x01, 0x00, 0x01,
    /* 20 bytes dummy rendezvous cookie */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13,
    /* 128 bytes dummy DH handshake data */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

static void do_decrypt_test(uint8_t *plaintext, size_t plaintext_len);
static void do_early_parse_test(uint8_t *plaintext, size_t plaintext_len);
static void do_late_parse_test(uint8_t *plaintext, size_t plaintext_len);
static void do_parse_test(uint8_t *plaintext, size_t plaintext_len, int phase);
static ssize_t make_intro_from_plaintext(
    void *buf, size_t len, crypto_pk_t *key, void **cell_out);

#define EARLY_PARSE_ONLY 1
#define DECRYPT_ONLY 2
#define ALL_PARSING 3

static void
do_early_parse_test(uint8_t *plaintext, size_t plaintext_len)
{
  do_parse_test(plaintext, plaintext_len, EARLY_PARSE_ONLY);
}

static void
do_decrypt_test(uint8_t *plaintext, size_t plaintext_len)
{
  do_parse_test(plaintext, plaintext_len, DECRYPT_ONLY);
}

static void
do_late_parse_test(uint8_t *plaintext, size_t plaintext_len)
{
  do_parse_test(plaintext, plaintext_len, ALL_PARSING);
}

/** Test utility function: checks that the <b>plaintext_len</b>-byte string at
 * <b>plaintext</b> is at least superficially parseable.
 */
static void
do_parse_test(uint8_t *plaintext, size_t plaintext_len, int phase)
{
  crypto_pk_t *k = NULL;
  ssize_t r;
  uint8_t *cell = NULL;
  size_t cell_len;
  rend_intro_cell_t *parsed_req = NULL;
  char *err_msg = NULL;
  char digest[DIGEST_LEN];

  /* Get a key */
  k = crypto_pk_new();
  tt_assert(k);
  r = crypto_pk_read_private_key_from_string(k, AUTHORITY_SIGNKEY_1, -1);
  tt_assert(!r);

  /* Get digest for future comparison */
  r = crypto_pk_get_digest(k, digest);
  tt_assert(r >= 0);

  /* Make a cell out of it */
  r = make_intro_from_plaintext(
      plaintext, plaintext_len,
      k, (void **)(&cell));
  tt_assert(r > 0);
  tt_assert(cell);
  cell_len = r;

  /* Do early parsing */
  parsed_req = rend_service_begin_parse_intro(cell, cell_len, 2, &err_msg);
  tt_assert(parsed_req);
  tt_ptr_op(err_msg, OP_EQ, NULL);
  tt_mem_op(parsed_req->pk,OP_EQ, digest, DIGEST_LEN);
  tt_assert(parsed_req->ciphertext);
  tt_assert(parsed_req->ciphertext_len > 0);

  if (phase == EARLY_PARSE_ONLY)
    goto done;

  /* Do decryption */
  r = rend_service_decrypt_intro(parsed_req, k, &err_msg);
  tt_assert(!r);
  tt_ptr_op(err_msg, OP_EQ, NULL);
  tt_assert(parsed_req->plaintext);
  tt_assert(parsed_req->plaintext_len > 0);

  if (phase == DECRYPT_ONLY)
    goto done;

  /* Do late parsing */
  r = rend_service_parse_intro_plaintext(parsed_req, &err_msg);
  tt_assert(!r);
  tt_ptr_op(err_msg, OP_EQ, NULL);
  tt_assert(parsed_req->parsed);

 done:
  tor_free(cell);
  crypto_pk_free(k);
  rend_service_free_intro(parsed_req);
  tor_free(err_msg);
}

/** Given the plaintext of the encrypted part of an INTRODUCE1/2 and a key,
 * construct the encrypted cell for testing.
 */

static ssize_t
make_intro_from_plaintext(
    void *buf, size_t len, crypto_pk_t *key, void **cell_out)
{
  char *cell = NULL;
  ssize_t cell_len = -1, r;
  /* Assemble key digest and ciphertext, then construct the cell */
  ssize_t ciphertext_size;

  if (!(buf && key && len > 0 && cell_out)) goto done;

  /*
   * Figure out an upper bound on how big the ciphertext will be
   * (see crypto_pk_obsolete_public_hybrid_encrypt())
   */
  ciphertext_size = PKCS1_OAEP_PADDING_OVERHEAD;
  ciphertext_size += crypto_pk_keysize(key);
  ciphertext_size += CIPHER_KEY_LEN;
  ciphertext_size += len;

  /*
   * Allocate space for the cell
   */
  cell = tor_malloc(DIGEST_LEN + ciphertext_size);

  /* Compute key digest (will be first DIGEST_LEN octets of cell) */
  r = crypto_pk_get_digest(key, cell);
  tt_assert(r >= 0);

  /* Do encryption */
  r = crypto_pk_obsolete_public_hybrid_encrypt(
      key, cell + DIGEST_LEN, ciphertext_size,
      buf, len,
      PK_PKCS1_OAEP_PADDING, 0);
  tt_assert(r >= 0);

  /* Figure out cell length */
  cell_len = DIGEST_LEN + r;

  /* Output the cell */
  *cell_out = cell;
  cell = NULL;

 done:
  tor_free(cell);
  return cell_len;
}

/** Test v0 INTRODUCE2 parsing through decryption only
 */

static void
test_introduce_decrypt_v0(void *arg)
{
  (void)arg;
  do_decrypt_test(v0_test_plaintext, sizeof(v0_test_plaintext));
}

/** Test v1 INTRODUCE2 parsing through decryption only
 */

static void
test_introduce_decrypt_v1(void *arg)
{
  (void)arg;
  do_decrypt_test(v1_test_plaintext, sizeof(v1_test_plaintext));
}

/** Test v2 INTRODUCE2 parsing through decryption only
 */

static void
test_introduce_decrypt_v2(void *arg)
{
  (void)arg;
  do_decrypt_test(v2_test_plaintext, sizeof(v2_test_plaintext));
}

/** Test v3 INTRODUCE2 parsing through decryption only
 */

static void
test_introduce_decrypt_v3(void *arg)
{
  (void)arg;
  do_decrypt_test(
      v3_no_auth_test_plaintext, sizeof(v3_no_auth_test_plaintext));
  do_decrypt_test(
      v3_basic_auth_test_plaintext, sizeof(v3_basic_auth_test_plaintext));
}

/** Test v0 INTRODUCE2 parsing through early parsing only
 */

static void
test_introduce_early_parse_v0(void *arg)
{
  (void)arg;
  do_early_parse_test(v0_test_plaintext, sizeof(v0_test_plaintext));
}

/** Test v1 INTRODUCE2 parsing through early parsing only
 */

static void
test_introduce_early_parse_v1(void *arg)
{
  (void)arg;
  do_early_parse_test(v1_test_plaintext, sizeof(v1_test_plaintext));
}

/** Test v2 INTRODUCE2 parsing through early parsing only
 */

static void
test_introduce_early_parse_v2(void *arg)
{
  (void)arg;
  do_early_parse_test(v2_test_plaintext, sizeof(v2_test_plaintext));
}

/** Test v3 INTRODUCE2 parsing through early parsing only
 */

static void
test_introduce_early_parse_v3(void *arg)
{
  (void)arg;
  do_early_parse_test(
      v3_no_auth_test_plaintext, sizeof(v3_no_auth_test_plaintext));
  do_early_parse_test(
      v3_basic_auth_test_plaintext, sizeof(v3_basic_auth_test_plaintext));
}

/** Test v0 INTRODUCE2 parsing
 */

static void
test_introduce_late_parse_v0(void *arg)
{
  (void)arg;
  do_late_parse_test(v0_test_plaintext, sizeof(v0_test_plaintext));
}

/** Test v1 INTRODUCE2 parsing
 */

static void
test_introduce_late_parse_v1(void *arg)
{
  (void)arg;
  do_late_parse_test(v1_test_plaintext, sizeof(v1_test_plaintext));
}

/** Test v2 INTRODUCE2 parsing
 */

static void
test_introduce_late_parse_v2(void *arg)
{
  (void)arg;
  do_late_parse_test(v2_test_plaintext, sizeof(v2_test_plaintext));
}

/** Test v3 INTRODUCE2 parsing
 */

static void
test_introduce_late_parse_v3(void *arg)
{
  (void)arg;
  do_late_parse_test(
      v3_no_auth_test_plaintext, sizeof(v3_no_auth_test_plaintext));
  do_late_parse_test(
      v3_basic_auth_test_plaintext, sizeof(v3_basic_auth_test_plaintext));
}

#define INTRODUCE_LEGACY(name) \
  { #name, test_introduce_ ## name , 0, NULL, NULL }

struct testcase_t introduce_tests[] = {
  INTRODUCE_LEGACY(early_parse_v0),
  INTRODUCE_LEGACY(early_parse_v1),
  INTRODUCE_LEGACY(early_parse_v2),
  INTRODUCE_LEGACY(early_parse_v3),
  INTRODUCE_LEGACY(decrypt_v0),
  INTRODUCE_LEGACY(decrypt_v1),
  INTRODUCE_LEGACY(decrypt_v2),
  INTRODUCE_LEGACY(decrypt_v3),
  INTRODUCE_LEGACY(late_parse_v0),
  INTRODUCE_LEGACY(late_parse_v1),
  INTRODUCE_LEGACY(late_parse_v2),
  INTRODUCE_LEGACY(late_parse_v3),
  END_OF_TESTCASES
};
