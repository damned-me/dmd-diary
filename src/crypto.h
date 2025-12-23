/*
 * crypto.h - Encryption/decryption operations
 */
#ifndef CRYPTO_H
#define CRYPTO_H

#include "dry.h"

/* 
 * Mount or unmount encrypted diary
 * opcl: 0 = open (mount), 1 = close (unmount)
 */
void encdiary(int opcl, const char *name, const char *path);

#endif /* CRYPTO_H */
