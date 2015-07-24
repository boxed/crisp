
/*
 * =====[ IDEA.C ]=========================================================
 *
 * Description:     Algorithm evaluation.
 *
 * Notes:           Comments are restructured, the file is customised to the
 * expected high standards. Defines are converted to upper case. The
 * resulting object code compares out with the original O.K. (functionality
 * intact!)
 *
 *
 *
 * Revisions:
 *
 *
 *  REV     DATE     BY           DESCRIPTION
 *  ----  --------  ----------  --------------------------------------
 *  0.00  01.01.95  Peter Glen  Initial version.
 *  0.00  31.08.93  Peter Glen  Initial version of modification.
 *  0.00  10.02.95  Peter Glen  Library define removed.
 *  0.00  18.05.95  Peter Glen  WIN 95 initial version.
 *
 * =======================================================================
 */

/*
 * IDEA.C - C source code for IDEA block cipher. IDEA (International Data
 * Encryption Algorithm), formerly known as IPES (Improved Proposed
 * Encryption Standard). Algorithm developed by Xuejia Lai and James L.
 * Massey, of ETH Zurich. This implementation modified and derived from
 * original C code developed by Xuejia Lai.  Last modified 8 July 92.
 *
 * Zero-based indexing added, names changed from IPES to IDEA. CFB functions
 * added. Random number routines added.
 *
 * The IDEA(tm) block cipher is covered by a patent held by ETH and a Swiss
 * company called Ascom-Tech AG. The Swiss patent number is PCT/CH91/00117.
 * International patents are pending. IDEA(tm) is a trademark of Ascom-Tech
 * AG.  There is no license fee required for noncommercial use.  Commercial
 * users may obtain licensing details from Dieter Profos, Ascom Tech AG,
 * Solothurn Lab, Postfach 151, 4502 Solothurn, Switzerland, Tel +41 65
 * 242885, Fax +41 65 235761.
 *
 * The IDEA block cipher uses a 64-bit block size, and a 128-bit key size.  It
 * breaks the 64-bit cipher block into four 16-bit words because all of the
 * primitive inner operations are done with 16-bit arithmetic.  It likewise
 * breaks the 128-bit cipher key into eight 16-bit words.
 *
 * For further information on the IDEA cipher, see these papers: 1) Xuejia Lai,
 * "Detailed Description and a Software Implementation of the IPES Cipher",
 * Institute for Signal and Information Processing, ETH-Zentrum, Zurich,
 * Switzerland, 1991 2) Xuejia Lai, James L. Massey, Sean Murphy, "Markov
 * Ciphers and Differential Cryptanalysis", Advances in Cryptology-
 * EUROCRYPT'91
 *
 * This code assumes that each pair of 8-bit bytes comprising a 16-bit word in
 * the key and in the cipher block are externally represented with the Most
 * Significant Byte (MSB) first, regardless of the internal native byte order
 * of the target CPU.
 *
 */

/* #define lower16(x) ((word16)((x) & MASK16)) *//* unoptimized version */
#define lower16(x)   ((word16)(x))

#define MAXIM        0x10001
#define FUJI         0x10000
#define MASK16       ((word16) 0xffff)
#define ROUNDS       8

static word16 mul(REG word16 a, REG word16 b);
static word16 inv(word16 x);
static void en_key_idea(short  userkey[8], //JK
                             word16  Z[6][ROUNDS + 1]);
//JK
static void de_key_idea(word16  Z[6][ROUNDS + 1], //JK
                             word16  DK[6][ROUNDS + 1]);
//JK
static void cipher_idea(short  inblk[4], short  outblk[4], //JK
                             REG word16  Z[6][ROUNDS + 1]);
//JK
static void xorbuf(REG char  * buf, char  * mask, REG int count);
//JK
static void cfbshift(REG char  * iv, REG char  * buf, REG int count, //JK
                          int blocksize);

/*
 * Multiplication, modulo (2**16)+1
 */

static word16 mul(REG word16 a, REG word16 b)

{
    REG word32 q;
    REG long int p;
    if (a == 0)
        p = MAXIM - b;
    else
    if (b == 0)
        p = MAXIM - a;
    else
        {
        q = (word32) a *(word32) b;
        p = (q & MASK16) - (q >> 16);
        if (p <= 0)
            p = p + MAXIM;
        }
    return (lower16(p));
}                               /* mul */

/*
 * Compute multiplicative inverse of x, modulo (2**16)+1, using Euclid's GCD
 * algorithm.
 */

static word16 inv(word16 x)

{
    long n1, n2, q, r, b1, b2, t;
    if (x == 0)
        b2 = 0;
    else
        {
        n1 = MAXIM;
        n2 = x;
        b2 = 1;
        b1 = 0;
        do
            {
            r = (n1 % n2);
            q = (n1 - r) / n2;
            if (r == 0)
                {
                if (b2 < 0)
                    b2 = MAXIM + b2;
                } else
                {
                n1 = n2;
                n2 = r;
                t = b2;
                b2 = b1 - q * b2;
                b1 = t;
                }
            } while (r != 0);
        }
#ifdef IDEA_DEBUG
    if (mul(x, (word16) b2) != 1)       /* check answer */
        printf("\n\07Error! inv(%u) = %u ?\n", x, (word16) b2);
#endif
    return ((word16) b2);
}                               /* inv */


/*
 * Compute IDEA encryption subkeys Z
 */

static void en_key_idea(short  userkey[8], word16  Z[6][ROUNDS + 1]) //JK

{
    word16 S[54];
    int  i, j, r;
/* shifts   */
    for (i = 0; i < 8; i++)
        S[i] = userkey[i];
    for (i = 8; i < 54; i++)
        {
        if ((i + 2) % 8 == 0)   /* for S[14],S[22],...  */
            S[i] = lower16((S[i - 7] << 9) ^ (S[i - 14] >> 7));
        else
        if ((i + 1) % 8 == 0)   /* for S[15],S[23],...   */
            S[i] = lower16((S[i - 15] << 9) ^ (S[i - 14] >> 7));
        else
            S[i] = lower16((S[i - 7] << 9) ^ (S[i - 6] >> 7));
        }
/* get subkeys */
    for (r = 0; r < ROUNDS + 1; r++)
        for (j = 0; j < 6; j++)
            Z[j][r] = S[6 * r + j];

/* clear sensitive key data from memory... */
    for (i = 0; i < 54; i++)
        S[i] = 0;

}                               /* en_key_idea */

/*
 * Compute IDEA decryption subkeys DK from encryption subkeys Z
 */

static void de_key_idea(word16  Z[6][ROUNDS + 1], //JK
                             word16  DK[6][ROUNDS + 1]) // JK

{
    int  j;
    for (j = 0; j < ROUNDS + 1; j++)
        {
        DK[0][ROUNDS - j] = inv(Z[0][j]);
        DK[3][ROUNDS - j] = inv(Z[3][j]);
        if (j == 0 || j == ROUNDS)
            {
            DK[1][ROUNDS - j] = lower16(FUJI - Z[1][j]);
            DK[2][ROUNDS - j] = lower16(FUJI - Z[2][j]);
            } else
            {
            DK[1][ROUNDS - j] = lower16(FUJI - Z[2][j]);
            DK[2][ROUNDS - j] = lower16(FUJI - Z[1][j]);
            }
        }
    for (j = 0; j < ROUNDS; j++)
        {
        DK[4][ROUNDS - 1 - j] = Z[4][j];
        DK[5][ROUNDS - 1 - j] = Z[5][j];
        }
}                               /* de_key_idea */

/*
 * IDEA encryption/decryption algorithm
 *
 * Note that inblk and outblk can be the same buffer.
 */

static void cipher_idea(short  inblk[4], short  outblk[4], //JK
                             REG word16  Z[6][ROUNDS + 1]) //JK

{
    int  r;
    REG word16 x1, x2, x3, x4, kk, t1, t2, a;
    x1 = inblk[0];
    x2 = inblk[1];
    x3 = inblk[2];
    x4 = inblk[3];
    for (r = 0; r < ROUNDS; r++)
        {
        x1 = mul(x1, Z[0][r]);
        x4 = mul(x4, Z[3][r]);
        x2 = lower16(x2 + Z[1][r]);
        x3 = lower16(x3 + Z[2][r]);
        kk = mul(Z[4][r], lower16(x1 ^ x3));
        t1 = mul(Z[5][r], lower16(kk + (x2 ^ x4)));
        t2 = lower16(kk + t1);
        x1 = x1 ^ t1;
        x4 = x4 ^ t2;
        a = x2 ^ t2;
        x2 = x3 ^ t1;
        x3 = a;
        }
    outblk[0] = mul(x1, Z[0][ROUNDS]);
    outblk[3] = mul(x4, Z[3][ROUNDS]);
    outblk[1] = lower16(x3 + Z[1][ROUNDS]);
    outblk[2] = lower16(x2 + Z[2][ROUNDS]);
}                               /* cipher_idea */

#define byteglue(lo,hi) ((((word16) hi) << 8) + (word16) lo)

/*
 * XORBUF - change buffer via xor with random mask block Used for Cipher
 * Feedback (CFB) or Cipher Block Chaining (CBC) modes of encryption. Can be
 * applied for any block encryption algorithm, with any block size, such as
 * the DES or the IDEA cipher. count must be > 0
 *
 */

static void xorbuf(REG char  * buf, REG char  * mask, REG int count) //JK
{
    if  (count)
        do
               * buf++ ^= *mask++;
        while (--count);
}                               /* xorbuf */

/*
 * CFBSHIFT - shift bytes into IV for CFB input. Used only for Cipher
 * Feedback (CFB) mode of encryption. Can be applied for any block encryption
 * algorithm with any block size, such as the DES or the IDEA cipher.
 */

static void cfbshift(REG char  * iv, REG char  * buf, REG int count, //JK
                          int blocksize)

/*
 * iv is the initialization vector. buf is the buffer pointer. count is the
 * number of bytes to shift in...must be > 0. blocksize is 8 bytes for DES or
 * IDEA ciphers.
 */

{
    int  retained;
    if (count)
        {
        retained = blocksize - count;   /* number bytes in iv to retain */
/* left-shift retained bytes of IV over by count bytes to make room */
        while (retained--)
            {
            *iv = *(iv + count);
            iv++;
            }
/* now copy count bytes from buf to shifted tail of IV */
        do
            *iv++ = *buf++;
        while (--count);
        }
}                               /* cfbshift */

/*
 * Key schedules for IDEA encryption and decryption
 */

static word16  Z[6][ROUNDS + 1], DK[6][ROUNDS + 1];
//JK
static word16( * keyschedule)[ROUNDS + 1];           /* JK pointer to
                                                         * keyschedule Z, DK */
static word16  *iv_idea;
//JK                            /* pointer to IV for CFB */
static boolean cfb_dc_idea;     /* TRUE iff CFB decrypting */

/* ----------------------------------------------------------------------- */

#ifdef TEST

void main(void)
{                               /* Test driver for IDEA cipher */
    int  i, j, k, err = FALSE;
    word16 Z[6][ROUNDS + 1], DK[6][ROUNDS + 1], XX[4], TT[4], YY[4];
    word16 userkey[8];
    word16 iv[4];


    for (i = 0; i < 4; i++)     /* clear iv buffer */
        iv[i] = 0;
/* Make a sample user key for testing... */
    for (i = 0; i < 8; i++)
        userkey[i] = i + 1;

/* Compute encryption subkeys from user key... */
    en_key_idea(userkey, Z);

/*
 * printf("\nEncryption key subblocks: "); for (j = 0; j < ROUNDS + 1; j++) {
 * printf("\nround %d:   ", j + 1); if (j == ROUNDS) for (i = 0; i < 4; i++)
 * printf(" %6u", Z[i][j]); else for (i = 0; i < 6; i++) printf(" %6u",
 * Z[i][j]); }
 */
/* Compute decryption subkeys from encryption subkeys... */
    de_key_idea(Z, DK);
/*
 * printf("\nDecryption key subblocks: "); for (j = 0; j < ROUNDS + 1; j++) {
 * printf("\nround %d:   ", j + 1); if (j == ROUNDS) for (i = 0; i < 4; i++)
 * printf(" %6u", DK[i][j]); else for (i = 0; i < 6; i++) printf(" %6u",
 * DK[i][j]); }
 */
/* Make a sample plaintext pattern for testing... */
    for (k = 0; k < 4; k++)
        XX[k] = k;

    cipher_idea(XX, YY, Z);     /* encrypt plaintext XX, making YY */
    cipher_idea(YY, TT, DK);    /* decrypt ciphertext YY, making TT */

    printf("\nEncription using original:\n");

    printf("\nX %6u   %6u  %6u  %6u \n",
           XX[0], XX[1], XX[2], XX[3]);
    printf("Y %6u   %6u  %6u  %6u \n",
           YY[0], YY[1], YY[2], YY[3]);
    printf("T %6u   %6u  %6u  %6u \n",
           TT[0], TT[1], TT[2], TT[3]);

    for (k = 0; k < 4; k++)
        if (TT[k] != XX[k])
            err = TRUE;

/* PART 2 */

/* Make a sample plaintext pattern for testing... */
    for (k = 0; k < 4; k++)
        XX[k] = k;

    initkey_idea(userkey, FALSE);
    idea_ecb(XX, YY);           /* encrypt plaintext XX, making YY */

    initkey_idea(userkey, TRUE);
    idea_ecb(YY, TT);           /* decrypt ciphertext YY, making TT */

    printf("\nEncription using interface:\n");

    printf("\nX %6u   %6u  %6u  %6u \n",
           XX[0], XX[1], XX[2], XX[3]);
    printf("Y %6u   %6u  %6u  %6u \n",
           YY[0], YY[1], YY[2], YY[3]);
    printf("T %6u   %6u  %6u  %6u \n",
           TT[0], TT[1], TT[2], TT[3]);

    for (k = 0; k < 4; k++)
        if (TT[k] != XX[k])
            err = TRUE;

/* PART 3 */

/* Make a sample plaintext pattern for testing... */
    for (k = 0; k < 4; k++)
        XX[k] = k;

    for (k = 0; k < 4; k++)
        YY[k] = k;

    for (k = 0; k < 4; k++)
        TT[k] = k;

    for (i = 0; i < 4; i++)     /* clear iv buffer */
        iv[i] = 0;

    initcfb_idea(iv, userkey, FALSE);
    ideacfb(YY, 8);

    for (i = 0; i < 4; i++)     /* clear iv buffer */
        iv[i] = 0;

    initcfb_idea(iv, userkey, FALSE);
    ideacfb(TT, 8);

    for (i = 0; i < 4; i++)     /* clear iv buffer */
        iv[i] = 0;

    initcfb_idea(iv, userkey, TRUE);
    ideacfb(TT, 8);

    printf("\nEncription using chained original:\n");

    printf("\nX %6u   %6u  %6u  %6u \n",
           XX[0], XX[1], XX[2], XX[3]);
    printf("Y %6u   %6u  %6u  %6u \n",
           YY[0], YY[1], YY[2], YY[3]);
    printf("T %6u   %6u  %6u  %6u \n",
           TT[0], TT[1], TT[2], TT[3]);

    for (k = 0; k < 4; k++)
        if (TT[k] != XX[k])
            err = TRUE;
    if (err)
        {
        printf("\n\07Error!  Noninvertable encryption.\n");
        }
    exit(0);                    /* normal exit */
}                               /* main */

#endif

/* ----------------------------------------------------------------------- */

/*
 * initkey_idea initializes IDEA for ECB mode operations
 *
 */

void initkey_idea(char  key[16], boolean decryp) //JK

{
    word16 userkey[8];          /* IDEA key is 16 bytes long */
    int  i;
/* Assume each pair of bytes comprising a word is ordered MSB-first. */

    for (i = 0; i < 8; i++)
        {
        userkey[i] = *((word16  *) key);
        //JK, sonst Seg.lost SS != DS

/* userkey[i] = byteglue(*(key + 1), *key); */
            key++;
        key++;
        }

    en_key_idea(userkey, Z);
    keyschedule = Z;
    if (decryp)
        {
        int  r, j;
        de_key_idea(Z, DK);     /* compute inverse key schedule DK */
        keyschedule = DK;
/* Don't need Z anymore.  Erase dangerous traces... */
        for (r = 0; r < ROUNDS + 1; r++)
            for (j = 0; j < 6; j++)
                Z[j][r] = 0;
        }
    for (i = 0; i < 8; i++)     /* Erase dangerous traces */
        userkey[i] = 0;
}                               /* initkey_idea */

/*
 * Run a 64-bit block thru IDEA in ECB (Electronic Code Book) mode, using the
 * currently selected key schedule.
 */

void idea_ecb(short  * inbuf, short  * outbuf)

{
/* Assume each pair of bytes comprising a word is ordered MSB-first. */
#ifndef HIGHFIRST
/* Invert the byte order for each 16-bit word for internal use. */
    union
        {
        word16 w[4];
        byte b[8];
        }    inbuf2, outbuf2;
    REG byte  *p;
    //JK
        p = (byte  *) inbuf;
    //JK
        inbuf2.b[1] = *p++;
    inbuf2.b[0] = *p++;
    inbuf2.b[3] = *p++;
    inbuf2.b[2] = *p++;
    inbuf2.b[5] = *p++;
    inbuf2.b[4] = *p++;
    inbuf2.b[7] = *p++;
    inbuf2.b[6] = *p++;
    cipher_idea(inbuf2.w, outbuf2.w, keyschedule);
/* Now invert the byte order for each 16-bit word for external use. */
    p = (byte  *) outbuf;
    *p++ = outbuf2.b[1];
    *p++ = outbuf2.b[0];
    *p++ = outbuf2.b[3];
    *p++ = outbuf2.b[2];
    *p++ = outbuf2.b[5];
    *p++ = outbuf2.b[4];
    *p++ = outbuf2.b[7];
    *p++ = outbuf2.b[6];
#else
/* Byte order for internal and external representations is the same. */
         cipher_idea(inbuf, outbuf, keyschedule);
#endif
}                               /* idea_ecb */

/*
 * INITCFB - Initializes the IDEA key schedule tables via key, and
 * initializes the Cipher Feedback mode IV. References context variables
 * cfb_dc_idea and iv_idea.
 */

void initcfb_idea(word16  iv0[4], char  key[16], boolean decryp)

/*
 * iv0 is copied to global iv_idea, buffer will be destroyed by ideacfb. key
 * is pointer to key buffer. decryp is TRUE if decrypting, FALSE if
 * encrypting.
 */
{
         iv_idea = iv0;
    cfb_dc_idea = decryp;
    initkey_idea(key, FALSE);
}                               /* initcfb_idea */

/*
 * IDEACFB - encipher a buffer with IDEA enciphering algorithm, using Cipher
 * Feedback (CFB) mode.
 *
 * Assumes initcfb_idea has already been called. References context variables
 * cfb_dc_idea and iv_idea.
 *
 */

void ideacfb(char  * buf, int count) //JK , da direkt aus Anwendung ! !!

/*
 * buf is input, output buffer, may be more than 1 block. count is byte count
 * of buffer. May be > IDEABLOCKSIZE.
 */

{
    int  chunksize;             /* smaller of count, IDEABLOCKSIZE */
    word16  temp[IDEABLOCKSIZE / 2];
    //JK

        while ((chunksize = min(count, IDEABLOCKSIZE)) > 0)
        {
        idea_ecb(iv_idea, temp);/* encrypt iv_idea, making temp. */

        if (cfb_dc_idea)        /* buf is ciphertext */
/* shift in ciphertext to IV... */
            cfbshift((byte  *) iv_idea, buf, chunksize, IDEABLOCKSIZE);
        //JK

/* convert buf via xor */
/* buf now has enciphered output */
            xorbuf(buf, (byte  *) temp, chunksize);
        //JK

            if (!cfb_dc_idea)   /* buf was plaintext, is now ciphertext */
/* shift in ciphertext to IV... */
            cfbshift((byte  *) iv_idea, buf, chunksize, IDEABLOCKSIZE);
        //JK

            count -= chunksize;
        buf += chunksize;
        }
}                               /* ideacfb */

/*
 * close_idea function erases all the key schedule information when we are
 * all done with a set of operations for a particular IDEA key context. This
 * is to prevent any sensitive data from being left around in memory.
 *
 * Erase current key schedule tables.
 *
 */

void close_idea(void)

{
    short r, j;
    for (r = 0; r < ROUNDS + 1; r++)
        for (j = 0; j < 6; j++)
            keyschedule[j][r] = 0;
}                               /* close_idea() */


/* --------- END OF IDEA.C ----------------------------------------------- */
