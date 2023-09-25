/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return ~(~(~x & y) & ~(x & ~y));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int num = x + 1; 
  return !!num & !(num ^ (~num + 1));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int a = 0xAA | 0xAA << 8;
  a = a | a << 16;
  return !(a & ~x);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  return !((x ^ 0x30) >> 3) | !((x ^ 0x38) >> 1);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  return (y & !!x << 31 >> 31) | (z & !x << 31 >> 31);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int a = x >> 31;
  int b = ~y >> 31; 
  int con1 = a | b;
  int con2 = a & b;
  int dif = y + ~x + 1;
  return (!!con1 & !(dif >> 31)) | !!con2;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  int num = x ^ (~x + 1);
  int num1 = 1 & num >> 31;
  return (1 ^ num1) & ~x >> 31;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
/*
  int i1, i2, i3, i4, i34, i24;
  int j1, j2, j3, j4;
  int k1, k2, k3, k4, k5, k6, k7, k8;
  int l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12, l13, l14, l15, l16;
  int num1, num2, num3, num4, num5;
  int res, res1, res2, res3, res4, res5, res10, res20, res30, res40, res12, res13, res14, res23, res24, res34, res123, res124, res134, res234, res1234;
  
  x = x ^ x >> 31;

  i1 = 0xFF;
  i2 = i1 << 8;
  i3 = i1 << 16;
  i4 = i1 << 24;

  i34 = i3 | i4;
  num1 = x & i34;
  res1 = 1 & !!num1;
  res10 = !res1 << 31 >> 31;

  i24 = i2 | i4;
  num2 = x & i4 | x & i2 & res10;
  res2 = 1 & !!num2;
  res20 = !res2 << 31 >> 31;
  res12 = res10 & res20;

  j1 = 0xF0;
  j2 = j1 << 8;
  j3 = j1 << 16;
  j4 = j1 << 24;
  num3 = x & j4 | x & j3 & res20 | x & j2 & res10 | x & j1 & res12;
  res3 = 1 & !!num3;
  res30 = !res3 << 31 >> 31;
  res13 = res10 & res30;
  res23 = res20 & res30;
  res123 = res12 & res30;

  k1 = 0xC;
  k2 = k1 << 4;
  k3 = k1 << 8;
  k4 = k1 << 12;
  k5 = k1 << 16;
  k6 = k1 << 20;
  k7 = k1 << 24;
  k8 = k1 << 28;
  num4 = x & k8 | x & k7 & res30 | x & k6 & res20 | x & k5 & res23 | x & k4 & res10 | x & k3 & res13 | x & k2 & res12 | x & k1 & res123;
  res4 = 1 & !!num4;
  res40 = !res4 << 31 >> 31;
  res14 = res10 & res40;
  res24 = res20 & res40;
  res34 = res30 & res40;
  res124 = res12 & res40;
  res134 = res13 & res40;
  res234 = res23 & res40;
  res1234 = res123 & res40;

  l1 = 2;
  l2 = l1 << 2;
  l3 = l1 << 4;
  l4 = l1 << 6;
  l5 = l1 << 8;
  l6 = l1 << 10;
  l7 = l1 << 12;
  l8 = l1 << 14;
  l9 = l1 << 16;
  l10 = l1 << 18;
  l11 = l1 << 20;
  l12 = l1 << 22;
  l13 = l1 << 24;
  l14 = l1 << 26;
  l15 = l1 << 28;
  l16 = l1 << 30;
  num5 = x & l16 | x & l15 & res40 | x & l14 & res30 | x & l13 & res34 | x & l12 & res20 | x & l11 & res24 | x & l10 & res23 | x & l9 & res234 | x & l8 & res10 | x & l7 & res14 | x & l6 & res13 | x & l5 & res134 | x & l4 & res12 | x & l3 & res124 | x & l2 & res123 | x & l1 & res1234;
  res5 = 1 & !!num5;

  res = res1 << 4 | res2 << 3 | res3 << 2 | res4 << 1 | res5;

  return res + 1 + !!x;
*/
  int res;
  int num1, num2, num3, num4, num5;
  int res1, res2, res3, res4, res5;
  
  x = x ^ x >> 31;
  
  num1 = x >> 16;
  res1 = !!num1;
  
  x = x >> (res1 << 4);
  num2 = x >> 8;
  res2 = !!num2;
  
  x = x >> (res2 << 3);
  num3 = x >> 4;
  res3 = !!num3;
  
  x = x >> (res3 << 2);
  num4 = x >> 2;
  res4 = !!num4;
  
  x = x >> (res4 << 1);
  num5 = x >> 1;
  res5 = !!num5;
  
  res = res1 << 4 | res2 << 3 | res3 << 2 | res4 << 1 | res5;

  return res + 1 + !!x;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int sig = uf & 1 << 31; 
  int exp = uf >> 23 & 0xFF;
  int frac = uf << 9;
  
  int flag = exp ^ 0xFF && exp ^ 0xFE;
  if(!flag) {
  	if(!frac)
  		return sig | 0xFF << 23;
  	return uf;
  }
  if(!exp)
  	return sig | uf << 1;
  uf = (uf ^ exp << 23) | exp + 1 << 23;
  return uf;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  int sig = uf & 1 << 31; 
  int exp = uf >> 23 & 0xFF;
  int frac = uf << 9;
  int bias = 127;
  int exp1 = exp - bias;
  int res = frac >> 9 | 1 << 23;
  int res1 = res >> 23 - exp1;
  
  if(exp1 <= -1)
  	return 0;
  	
  if(exp1 > 31)
  	return 1 << 31;
  
  if(!sig)
  	return res1;
  return ~res1 + 1;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  if(x > 127)
  	return 0xFF << 23;
  if(x > -126)
  	return x + 127 << 23;
  if(x > -149)
  	return 1 << 149 + x;
  return 0;
}
