#include "print.h"

namespace
{
    constexpr s32 TRUNCATION_BUFFER_COUNT = 1024;
    thread_local s8 TruncationBuffer[TRUNCATION_BUFFER_COUNT];
}

void InlineReverse(s8 *Buffer, s32 First, s32 Last)
{
    for (s32 f = First, b = Last; f < b; f++, b--)
    {
        const s8 Temp = Buffer[f];
        Buffer[f] = Buffer[b];
        Buffer[b] = Temp;
    }
}

namespace
{
    void ResetBufferOnFailure(s8**Buffer, s32* BufLen, s8* OriginalBuffer, s32 OriginalBufLen)
    {
        for (s8* C = OriginalBuffer; C < *Buffer; ++C)
        {
            *C = '\0';
        }
        *Buffer = OriginalBuffer;
        *BufLen = OriginalBufLen;
    }
}

bool PrintInt(s8** Buffer, u64 Int, bool bNegative, s32* BufLen)
{
    s8* OriginalBuffer = *Buffer;
    s32 OriginalBufLen = *BufLen;
    s32 FirstDigitIndex = 0;
    bool bSuccess = true;

    if (bNegative)
    {
        PrintChar(Buffer, '-', BufLen);
        FirstDigitIndex = 1;
    }

    do
    {
        // I always do this instead of the divide + mod when I want remainders. I think it's faster but haven't tested it.
        // probably compilers will optimize to this at minimum, given optimization is >= -O1. It's nice when debug perf
        // runs fast though.
        u64 NextInt = Int / 10; 
        u8 Digit = Int - NextInt * 10; // same as u8 Digit = Int % 10;
        Int = NextInt;
        bSuccess = PrintChar(Buffer, '0' + Digit, BufLen);
    }
    while (Int > 0);

    if (!bSuccess)
    {
        ResetBufferOnFailure(Buffer, BufLen, OriginalBuffer, OriginalBufLen);
    }
    else
    {
        // * * * * * (buflen = 5)
        // 1 0 0 * * (after printing '100', buflen = 2)
        // 5 - 2 = 3 chars printed
        // 3 - 1 = 2, the index of the last character relative to the original buffer
        s32 LastDigitIndex = (OriginalBufLen - *BufLen) - 1;
        // digits are printed least to most significant -> need to reverse them
        InlineReverse(OriginalBuffer, FirstDigitIndex, LastDigitIndex);
    }
    return bSuccess;
}

bool PrintFloat(s8 **Buffer, float64 Float, s32 *BufLen)
{
    s8* OriginalBuffer = *Buffer;
    s32 OriginalBufLen = *BufLen;
    s32 FirstDigitIndex = 0;
    bool bSuccess = true;

    const bool bNegative = Float < 0;
    if (bNegative)
    {
        bSuccess = PrintChar(Buffer, '-', BufLen);
        FirstDigitIndex = 1;
        Float *= -1.0;
    }

    // -->> just assuming %.3f for now
    const u64 DigitsBeforeDecimal = (u64)Float;
    const u64 DigitsAfterDecimal = (u64)Round(Float * 1000.0) % 1000;

    if (bSuccess)
    {
        bSuccess = PrintInt(Buffer, DigitsBeforeDecimal, false, BufLen);
    }
    if (bSuccess)
    {
        bSuccess = PrintChar(Buffer, '.', BufLen);
    }
    if (bSuccess && DigitsAfterDecimal < 100)
    {
        bSuccess = PrintChar(Buffer, '0', BufLen);
    }
    if (bSuccess && DigitsAfterDecimal < 10)
    {
        bSuccess = PrintChar(Buffer, '0', BufLen);
    }
    if (bSuccess)
    {
        bSuccess = PrintInt(Buffer, DigitsAfterDecimal, false, BufLen);
    }

    if (!bSuccess)
    {
        ResetBufferOnFailure(Buffer, BufLen, OriginalBuffer, OriginalBufLen);
    }
    return bSuccess;
}

bool PrintCString(s8** Buffer, const s8* Str, s32* BufLen)
{
    for (const s8* c = Str; *c; c++)
    {
        if (!PrintChar(Buffer, *(const u8*)c, BufLen))
        {
            return false;
        }
    }
    return true;
}

bool PrintTString(s8** Buffer, TString& Str, s32* BufLen)
{
    for (int i = 0; i < Str.Count; i++)
    {
        if (!PrintChar(Buffer, *(const u8*)&Str[i], BufLen))
        {
            return false;
        }
    }
    return false;
}

bool PrintTVector2(s8 **Buffer, TVector2 Vector, s32 *BufLen)
{
    s8* OriginalBuffer = *Buffer;
    s32 OriginalBufLen = *BufLen;
    s32 FirstDigitIndex = 0;

    bool bSuccess = PrintChar(Buffer, '<', BufLen);
    if (bSuccess)
    {
        bSuccess = PrintFloat(Buffer, Vector.X, BufLen);
    }
    if (bSuccess)
    {
        bSuccess = PrintCString(Buffer, ", ", BufLen);
    }
    if (bSuccess)
    {
        bSuccess = PrintFloat(Buffer, Vector.Y, BufLen);
    }
    if (bSuccess)
    {
        bSuccess = PrintChar(Buffer, '>', BufLen);
    }

    if (!bSuccess)
    {
        ResetBufferOnFailure(Buffer, BufLen, OriginalBuffer, OriginalBufLen);
    }
    return bSuccess;
}

TString SprintImpl(const s8 *Fmt, TAllocator *Allocator, FmtArg Args[], int ArgCount)
{
    constexpr int BUF_LEN = 8192;
    // leave room for escape codes.
    constexpr int END_CHAR_INDEX = BUF_LEN - 4; 
    s8 Buffer[BUF_LEN];

    s8* Buf = &Buffer[0];
    s32 Remain = END_CHAR_INDEX + 1;
    int ArgIndex = 0;

    // this probably isn't strictly correct but it should be fine for this project
    bool bBeginEscapeCode = false;
    for (const s8* c = Fmt; *c; ++c)
    {
        bool bFmtPrintFailure = false;
        if (bBeginEscapeCode)
        {
            PrintChar(&Buf, *(const u8*)c, &Remain);
            bBeginEscapeCode = false;
        }
        else if (*c == '\\')
        {
            PrintChar(&Buf, '\\', &Remain);
            bBeginEscapeCode = true;
        }
        else
        {
            if (*c == '%' && ArgIndex < ArgCount)
            {
                bool bSuccess = true;
                const FmtArg& Arg = Args[ArgIndex];
                switch (Arg.Type)
                {
                case EArgType::U8:
                    bSuccess = PrintInt(&Buf, Arg.U8, false, &Remain);
                    break;
                case EArgType::U16:
                    bSuccess = PrintInt(&Buf, Arg.U16, false, &Remain);
                    break;
                case EArgType::U32:
                    bSuccess = PrintInt(&Buf, Arg.U32, false, &Remain);
                    break;
                case EArgType::U64:
                    bSuccess = PrintInt(&Buf, Arg.U64, false, &Remain);
                    break;
                case EArgType::S8:
                    bSuccess = PrintInt(&Buf, abs(Arg.S8), Arg.S8 < 0, &Remain);
                    break;
                case EArgType::S16:
                    bSuccess = PrintInt(&Buf, abs(Arg.S16), Arg.S16 < 0, &Remain);
                    break;
                case EArgType::S32:
                    bSuccess = PrintInt(&Buf, abs(Arg.S32), Arg.S32 < 0, &Remain);
                    break;
                case EArgType::S64:
                    bSuccess = PrintInt(&Buf, abs(Arg.S64), Arg.S64 < 0, &Remain);
                    break;
                case EArgType::Float32:
                    bSuccess = PrintFloat(&Buf, Arg.Float32, &Remain);
                    break;
                case EArgType::Float64:
                    bSuccess = PrintFloat(&Buf, Arg.Float64, &Remain);
                    break;
                case EArgType::Character:
                    bSuccess = PrintChar(&Buf, Arg.Character, &Remain);
                    break;
                case EArgType::Pointer:
                    bSuccess = PrintInt(&Buf, (u64)Arg.Pointer, false, &Remain);
                    break;
                case EArgType::CString:
                    bSuccess = PrintCString(&Buf, Arg.CString, &Remain);
                    break;
                case EArgType::TString:
                    bSuccess = PrintTString(&Buf, *Arg.String, &Remain);
                    break;
                case EArgType::TVector2:
                    bSuccess = PrintTVector2(&Buf, Arg.Vector2, &Remain);
                    break;
                }
                bFmtPrintFailure = !bSuccess;
                ArgIndex++;
            }
            else
            {
                PrintChar(&Buf, *(const u8*)c, &Remain);
            }
            bBeginEscapeCode = false;
        }
        if (Remain <= 0 || bFmtPrintFailure)
        {
            break;
        }
    }

    Remain += 1; // there should always be room in the buffer for a null char
    PrintChar(&Buf, '\0', &Remain);

    return TString::Copy(Buffer, Allocator);
}

const s8 *TruncateWithEllipsis(const s8 *InStr, s32 MaxChars)
{
    MaxChars = MIN(MaxChars, TRUNCATION_BUFFER_COUNT - 3);
    assert(MaxChars > 2);

    s32 i = 0;
    while (true)
    {
        if (i == MaxChars-1)
        {
            if (InStr[i] != '\0' && InStr[i+1] != '\0')
            {
                TruncationBuffer[i] = '.';
                TruncationBuffer[i+1] = '.';
                TruncationBuffer[i+2] = '\0';
                break;
            }
        }
        TruncationBuffer[i] = InStr[i];
        if (InStr[i] == '\0')
        {
            break;
        }
        i++;
    }

    return TruncationBuffer;
}
