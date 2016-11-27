/* DEFINE.H - West Coast Controls Standard C Language Definitions */

#ifndef __DEFINE_H
#define __DEFINE_H


/* Type definitions */
typedef unsigned      char      BOOLEAN;
typedef unsigned      char      BYTE;
typedef signed        char      SBYTE;
typedef unsigned      int       WORD;
typedef signed        int       SWORD;
typedef unsigned long int       LWORD;
typedef signed   long int       SLWORD;

typedef unsigned char TFlag;
typedef float TData;

/* data types for Variants */
/* JLT added in Word and Boolean to the enum */
/* the lower byte of the enum has the number of bytes for the data type */
typedef enum {
  Empty       = 0x0000,
  Char        = 0x0100 | sizeof(char),
  UChar       = 0x0200 | sizeof(char),
  Byte        = 0x0300 | sizeof(char),
  Boolean     = 0x0400 | sizeof(char),
  Word        = 0x0500 | sizeof(unsigned int),
  Int         = 0x0600 | sizeof(int),
  UInt        = 0x0700 | sizeof(unsigned int),
  Long        = 0x0800 | sizeof(long),
  ULong       = 0x0900 | sizeof(unsigned long),
  Float       = 0x0a00 | sizeof(float),
  FloatPtr    = 0x0b00 | sizeof(float*),
  VoidPtr     = 0x0c00 | sizeof(void*),
  StringList  = 0x0d00 | sizeof(char*),
  UserDefined = 0x0e00 | 0x00,
  String      = 0x0f00 | 0x00,
  StringPtr   = 0x1000 | sizeof(char*),
  DataTypeSize = 0x00ff         /* mask for extracting data type size */
} TDataType;


typedef union _TMultiValue {
/* data place holders */
  char Char;
  unsigned char UChar;
  unsigned char Byte;               
  unsigned char Boolean;        /* Line added by JLT to explicitly include a "boolean" */       
  unsigned int Word;            /* Line added by JLT to explicitly include a "word" */
  int Int;
  unsigned int UInt;
  long Long;
  float Float;  
  double Double;                   
   

/* pointers to data place holders */
  unsigned char *bPtr;
  unsigned char *BoolPtr;       /* Line added by JLT to explicitly include a "boolean" */
  unsigned int *wPtr;           /* Line added by JLT to explicitly include a "word" */
  int *iPtr;
  int *uiPtr;
  float *fPtr;    
  double *dPtr; 
  void *Void;

/* arrays for data manipulation */
  unsigned char b[4];
  unsigned int w[2];
} TMultiValue;

typedef struct {
  TDataType DataType;
  TMultiValue Value;
} TVariant;

typedef struct {
  TDataType DataType;
  TMultiValue *Value;
} TVariantPtr;

/* used for working with limits */
typedef struct {
  BYTE Upper;                   /* upper limit */
  BYTE Lower;                   /* lower limit */
} TLimitB;

typedef struct {
  int Upper;                    /* upper limit */
  int Lower;                    /* lower limit */
} TLimitI;

typedef struct {
  TData Upper;                  /* upper limit */
  TData Lower;                  /* lower limit */
} TLimit;

/* used for state machines */
typedef enum {
  STATUS_FAILURE = 0,
  STATUS_SUCCESS = 1,
  STATUS_WORKING
} TStatus;

/* Boolean definitions */
#define TRUE    1
#define FALSE   0
#define YES     1
#define NO      0
#define ON      1
#define OFF     0

#define SUCCESS 0
#define FAILR   1   //*MH resolve redefinition conflict (namespace)
#define OPEN    0
#define CLOSED  1
#define DEBUGON   1
#define DEBUGOFF  0
#define VALID   1
#define INVALID 0
#define FINISHED 1
#define UNFINISHED 0

/* To comment out a function, so it's not linked in. */
#define UNUSED 0


/* Logical definitions */
#define AND     &&
#define OR      ||
#define NOT     !

/* Bit-masking operations */
#define BIT_AND         &
#define BIT_OR          |
#define BIT_NOT         ~

/* Error values */
#define NO_ERROR        0
#define YES_ERROR       1

/* character defines */
#define BLANK  0x20     /* blank ASCII character */
#ifndef EOF
#define EOF    -1
#endif
#define NEWLINE '\n'    /* End of line terminator */

/* macro expressions */
#define DIM(x)  ((int)(sizeof(x) / sizeof(x[0])))
#define TBD     ;               /* flag to fix something */
#endif
