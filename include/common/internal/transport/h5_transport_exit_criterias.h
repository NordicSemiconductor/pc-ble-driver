#pragma once

#include <sstream>

class ExitCriterias
{
  public:
    ExitCriterias() noexcept
        : ioResourceError(false)
        , close(false)
    {}
    ExitCriterias(const ExitCriterias &) = delete;
    virtual ~ExitCriterias()
    {}

    bool ioResourceError;
    bool close;

    virtual bool isFullfilled() const = 0;

    virtual void reset()
    {
        ioResourceError = false;
        close           = false;
    }

    virtual std::string toString()
    {
        std::stringstream info;
        info << "ioResourceError:" << ioResourceError << " close:" << close;
        return info.str();
    }
};

class StartExitCriterias : public ExitCriterias
{
  public:
    bool isOpened;

    StartExitCriterias() noexcept
        : ExitCriterias()
        , isOpened(false)
    {}
    StartExitCriterias(const StartExitCriterias &) = delete;

    bool isFullfilled() const override
    {
        return ioResourceError || close || isOpened;
    }

    void reset() override
    {
        ExitCriterias::reset();
        isOpened = false;
    }

    std::string toString() override
    {
        std::stringstream info;
        info << "state:START " << ExitCriterias::toString() << " isOpened:" << isOpened
             << " isFullfilled:" << isFullfilled();
        return info.str();
    }
};

class ResetExitCriterias : public ExitCriterias
{
  public:
    bool resetSent;
    bool resetWait;

    ResetExitCriterias() noexcept
        : ExitCriterias()
        , resetSent(false)
        , resetWait(false)
    {}
    ResetExitCriterias(const ResetExitCriterias &) = delete;

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

    std::string toString() override
    {
        std::stringstream info;
        info << "state:RESET " << ExitCriterias::toString() << " resetSent:" << resetSent
             << " resetWait:" << resetWait << " isFullfilled:" << isFullfilled();

        return info.str();
    }
};

class UninitializedExitCriterias : public ExitCriterias
{
  public:
    bool syncSent;
    bool syncRspReceived;

    UninitializedExitCriterias() noexcept
        : ExitCriterias()
        , syncSent(false)
        , syncRspReceived(false)
    {}
    UninitializedExitCriterias(const UninitializedExitCriterias &) = delete;

    bool isFullfilled() const override
    {
        return ioResourceError || close || (syncSent && syncRspReceived);
    }

    void reset() override
    {
        ExitCriterias::reset();
        syncSent        = false;
        syncRspReceived = false;
    }

    std::string toString() override
    {
        std::stringstream info;
        info << "state:UNINITIALIZED " << ExitCriterias::toString() << " syncSent:" << syncSent
             << " syncRspReceived:" << syncRspReceived << " isFullfilled:" << isFullfilled();
        return info.str();
    }
};

class InitializedExitCriterias : public ExitCriterias
{
  public:
    bool syncConfigSent;
    bool syncConfigRspReceived;

    InitializedExitCriterias() noexcept
        : ExitCriterias()
        , syncConfigSent(false)
        , syncConfigRspReceived(false)
    {}
    InitializedExitCriterias(const InitializedExitCriterias &) = delete;

    bool isFullfilled() const override
    {
        return ioResourceError || close || (syncConfigSent && syncConfigRspReceived);
    }

    void reset() override
    {
        ExitCriterias::reset();
        syncConfigSent        = false;
        syncConfigRspReceived = false;
    };

    std::string toString() override
    {
        std::stringstream info;
        info << "state:INITIALIZED " << ExitCriterias::toString()
             << " syncConfigSent:" << syncConfigSent
             << " syncConfigRspReceived:" << syncConfigRspReceived
             << " isFullfilled:" << isFullfilled();
        return info.str();
    }
};

class ActiveExitCriterias : public ExitCriterias
{
  public:
    bool irrecoverableSyncError;
    bool syncReceived;

    ActiveExitCriterias() noexcept
        : ExitCriterias()
        , irrecoverableSyncError(false)
        , syncReceived(false)
    {}
    ActiveExitCriterias(const ActiveExitCriterias &) = delete;

    bool isFullfilled() const override
    {
        return ioResourceError || close || syncReceived || irrecoverableSyncError;
    }

    void reset() override
    {
        ExitCriterias::reset();
        irrecoverableSyncError = false;
        syncReceived           = false;
    }

    std::string toString() override
    {
        std::stringstream info;
        info << "state:ACTIVE " << ExitCriterias::toString()
             << " irrecoverableSyncError:" << irrecoverableSyncError
             << " syncReceived:" << syncReceived << " isFullfilled:" << isFullfilled();

        return info.str();
    }
};
