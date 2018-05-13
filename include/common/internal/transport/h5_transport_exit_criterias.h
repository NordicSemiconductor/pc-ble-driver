/*
* Copyright (c) 2018 Nordic Semiconductor ASA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
*   1. Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
*   2. Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
*   3. Neither the name of Nordic Semiconductor ASA nor the names of other
*   contributors to this software may be used to endorse or promote products
*   derived from this software without specific prior written permission.
*
*   4. This software must only be used in or with a processor manufactured by Nordic
*   Semiconductor ASA, or in or with a processor manufactured by a third party that
*   is used in combination with a processor manufactured by Nordic Semiconductor.
*
*   5. Any software provided in binary or object form under this license must not be
*   reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef H5_TRANSPORT_EXIT_CRITERIAS_H
#define H5_TRANSPORT_EXIT_CRITERIAS_H

class ExitCriterias
{
public:
    ExitCriterias() noexcept
        : ioResourceError(false), close(false) {}
    virtual ~ExitCriterias() {}

    bool ioResourceError;
    bool close;

    virtual bool isFullfilled() const = 0;

    virtual void reset()
    {
        ioResourceError = false; close = false;
    }
};

class StartExitCriterias : public ExitCriterias
{
public:
    bool isOpened;

    StartExitCriterias() noexcept
        : ExitCriterias(),
        isOpened(false) {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || isOpened;
    }

    void reset() override
    {
        ExitCriterias::reset();
        isOpened = false;
    }
};

class UninitializedExitCriterias : public ExitCriterias
{
public:
    bool syncSent;
    bool syncRspReceived;

    UninitializedExitCriterias() noexcept
        : ExitCriterias(),
        syncSent(false),
        syncRspReceived(false) {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || (syncSent && syncRspReceived);
    }

    void reset() override
    {
        ExitCriterias::reset();
        syncSent = false;
        syncRspReceived = false;
    }
};

class InitializedExitCriterias : public ExitCriterias
{
public:
    bool syncConfigSent;
    bool syncConfigRspReceived;

    InitializedExitCriterias() noexcept
        : ExitCriterias(),
        syncConfigSent(false),
        syncConfigRspReceived(false) {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || (syncConfigSent && syncConfigRspReceived);
    }

    void reset() override
    {
        ExitCriterias::reset();
        syncConfigSent = false;
        syncConfigRspReceived = false;
    };

};

class ActiveExitCriterias : public ExitCriterias
{
public:
    bool irrecoverableSyncError;
    bool syncReceived;

    ActiveExitCriterias() noexcept
        : ExitCriterias(), irrecoverableSyncError(false),
        syncReceived(false) {}

    bool isFullfilled() const override {
        return ioResourceError || close || syncReceived || irrecoverableSyncError;
    }

    void reset() override
    {
        ExitCriterias::reset();
        irrecoverableSyncError = false;
        syncReceived = false;
    }
};

class ResetExitCriterias : public ExitCriterias
{
public:
    bool resetSent;
    bool resetWait;

    ResetExitCriterias() noexcept
        : ExitCriterias(), resetSent(false), resetWait(false)
    {}

    bool isFullfilled() const override
    {
        return ioResourceError || close || (resetSent && resetWait);
    }

    void reset() override
    {
        ExitCriterias::reset();
        resetSent = false;
        resetWait = false;
    }
};

#endif // H5_TRANSPORT_EXIT_CRITERIAS_H
