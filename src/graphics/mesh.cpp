#include "mesh.h"
#include "../core.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

namespace
{
    // returns file as temp-allocated string
    TString ReadFile(const TString& Path)
    {
        s32 File = open(Path.Data, O_RDONLY);
        if (File < 0)
        {
            return {};
        }

        struct stat FileStats;
        if (fstat(File, &FileStats))
        {
            close(File);
            return {};
        }

        const s64 FileSize = FileStats.st_size;

        TArray<s8> Bytes;
        Bytes.TempInit(FileSize);

        s32 Offset = 0;
        while (Offset < FileSize)
        {
            const s64 ReadCount = read(File, &Bytes[Offset], FileSize-Offset);
            if (ReadCount <= 0)
            {
                close(File);
                return {};
            }
            Offset += ReadCount;
        }

        close(File);
        return TString::WrapData(Bytes.Data(), Bytes.Count());
    }
}

bool Mesh::LoadObj(const TString &Path, TDynamicArray<TVector3>& OutVerts)
{
    assert(OutVerts.Count() == 0);
    s32 TempMark = Session.FrameAllocator.GetMark();

    TString FileBytes = ReadFile(Path);
    if (FileBytes.Count == 0)
    {
        return false;
    }

    OutVerts.Init(8192);
    TString Line = {};
    TString Remain = FileBytes.Duplicate();

    while (Remain.Count > 0)
    {
        Remain.Split('\n', Line, Remain);

        if (Line.Count == 0)
        {
            if (Remain.Count < 2)
            {
                break;
            }
            Line = Remain;
            Remain = {};
        }

        const s8 Char0 = ToLower(Line[0]);
        const s8 Char1 = ToLower(Line[1]);
        Line.Advance(2);

        if (Char0 == 'v' && Char1 == ' ')
        {
            TVector3& Vertex = OutVerts.Push();
            s32 ComponentIndex = 0;
            s32 PreDecimal = 0;
            s32 PostDecimal = 0;
            s32 PostDecimalPow10 = 1;
            float32 Sign = 1.0f;
            bool bFoundDecimal = false;
            bool bStartedParsingComponent = false;

            for (s32 i = 0; i < Line.Count; i++)
            {
                const s8 c = Line[i];
                if (c == ' ')
                {
                    if (!bStartedParsingComponent)
                    {
                        continue;
                    }
                }
                else
                {
                    bStartedParsingComponent = true;
                    if (c == '-')
                    {
                        Sign = -1.0f;
                    }
                    else if (c == '.')
                    {
                        bFoundDecimal = true;
                    }
                    else
                    {
                        const s32 Digit = c - 48;
                        if (bFoundDecimal)
                        {
                            PostDecimal = PostDecimal * 10 + Digit;
                            PostDecimalPow10 *= 10;
                        }
                        else
                        {
                            PreDecimal = PreDecimal * 10 + Digit;
                        }
                    }
                    if (i != Line.Count-1)
                    {
                        continue;
                    }
                }

                float32 Val = (float32)PreDecimal + (float32)PostDecimal / PostDecimalPow10;
                Val *= Sign;
                Vertex.SetComponent(ComponentIndex, Val);
                ComponentIndex++;

                PreDecimal = 0;
                PostDecimal = 0;
                Sign = 1;
                PostDecimalPow10 = 1;
                bFoundDecimal = false;
                bStartedParsingComponent = false;
            }
        }
    }

    LOG("loaded mesh %, vert count: %", Path.CStr(), OutVerts.Count());
    Session.FrameAllocator.ResetToMark(TempMark);
    return OutVerts.Count() > 0;
}