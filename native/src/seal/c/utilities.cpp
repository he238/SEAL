// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// STD
#include <algorithm>
#include <iterator>

// SEALNet
#include "seal/c/stdafx.h"
#include "seal/c/utilities.h"

// SEAL
#include "seal/context.h"
#include "seal/encryptionparams.h"
#include "seal/smallmodulus.h"
#include "seal/util/common.h"
#include "seal/util/locks.h"

using namespace std;
using namespace seal;
using namespace seal::c;
using namespace seal::util;

namespace seal
{
    namespace c
    {
        extern unordered_map<SEALContext *, shared_ptr<SEALContext>> pointer_store_;

        extern ReaderWriterLocker pointer_store_locker_;

        // This is here only so we have a null shared pointer to return.
        shared_ptr<SEALContext> null_context_;
    } // namespace c
} // namespace seal

unique_ptr<MemoryPoolHandle> seal::c::MemHandleFromVoid(void *voidptr)
{
    if (nullptr == voidptr)
    {
        return make_unique<MemoryPoolHandle>(MemoryManager::GetPool());
    }

    MemoryPoolHandle *handle = reinterpret_cast<MemoryPoolHandle *>(voidptr);
    return make_unique<MemoryPoolHandle>(*handle);
}

void seal::c::BuildSmallModulusPointers(const vector<SmallModulus> &in_mods, uint64_t *length, void **out_mods)
{
    *length = static_cast<uint64_t>(in_mods.size());
    if (out_mods == nullptr)
    {
        // The caller is only interested in the size
        return;
    }

    SmallModulus **mod_ptr_array = reinterpret_cast<SmallModulus **>(out_mods);
    transform(in_mods.begin(), in_mods.end(), mod_ptr_array, [](const auto &mod) { return new SmallModulus(mod); });
}

const shared_ptr<SEALContext> &seal::c::SharedContextFromVoid(void *context)
{
    SEALContext *ctx = FromVoid<SEALContext>(context);
    if (nullptr == ctx)
    {
        return null_context_;
    }

    ReaderLock lock(pointer_store_locker_.acquire_read());

    const auto &ctxiter = pointer_store_.find(ctx);
    if (ctxiter == pointer_store_.end())
    {
        return null_context_;
    }

    return ctxiter->second;
}

HRESULT seal::c::ToStringHelper(const string &str, char *outstr, uint64_t *length)
{
    uint64_t result_length = add_safe(static_cast<uint64_t>(str.length()), uint64_t(1));
    if (nullptr == outstr)
    {
        // We need to return the string length.
        // The string length will include the terminating character.
        *length = result_length;
        return S_OK;
    }

    // Verify the string fits
    if (*length < result_length)
    {
        *length = result_length;
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    fill_n(outstr, *length, char(0));
    str.copy(outstr, str.length());

    return S_OK;
}