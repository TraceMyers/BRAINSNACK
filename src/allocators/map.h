#pragma once

#include "array.h"
#include "../string.h"

// simple linear lookup map implementation. associate a key with a value. 
// doesn't template the key type, because keys are always treated as arbitrary strings of bytes.
// as with many things in this game, it is an incomplete representation that mostly just matches the needs of 
// the game, but is also overengineered a bit so as to demonstrate what I know how to do. (hire me!)
template<typename ValueType>
class TMap
{
public:

    struct FElement
    {
        u32 KeyHash;
        ValueType Value;
    };

    // returns true if the value was newly added, false if the value was replaced
    bool Set(const TString& Key, ValueType& Value)
    {
        const u32 Hash = GetHash(Key.Data, Key.Count);
        if (ValueType* ExistingValue = GetWithKeyHash(Hash))
        {
            *ExistingValue = Value;
            return false;
        }
        else
        {
            Elements.Push({Hash, Value});
            return true;
        }
    }

    // convenience wrapper for other Set(). wraps the Key with a TString (just a way to think of KeyType as arbitrary hashable bytes) and returns Set(TString& Key,..)
    // returns true if the value was newly added, false if the value was replaced
    // I honestly don't know cpp template language this well. chatgpt writes a lot of my template garbage for me.
    // credit to the engineers that fed it.
    template<typename KeyType>
    typename std::enable_if<!std::is_same<KeyType, TString>::value, bool>::type
    Set(const KeyType& Key, ValueType& Value)
    {
        TString KeyWrap = TString::Wrap(Key);
        return Set(KeyWrap, Value);
    }

    // returns nullptr if no element matches the key
    ValueType* Get(const TString& Key)
    {
        const u32 Hash = GetHash(Key.Data, Key.Count);
        return GetWithKeyHash(Hash);
    }

    // convenience operator for Get. returns nullptr if no element matches the key.
    // does *not* create a new item if the element doesn't exist in the map
    ValueType* operator[](const TString& Key)
    {
        return Get(Key);
    }

    // mostly just exists so both Set and Get can use it, allowing Set to avoid hashing twice,
    // but as an optimization you might carry around the hash of a thing for re-getting a lot.
    // returns nullptr if no element matches the key
    ValueType* GetWithKeyHash(u32 KeyHash)
    {
        for (s32 i = 0; i < Elements.Count(); i++)
        {
            if (Elements[i].KeyHash == KeyHash)
            {
                return &Elements[i].Value;
            }
        }
        return nullptr;
    }
    
    void Free()
    {
        Elements.Free();
    }

protected:

    TDynamicArray<FElement> Elements;
};