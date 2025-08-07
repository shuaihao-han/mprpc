#include "mprpccontroler.h"

MprpcController::MprpcController()
{
    m_failed = false;
    m_errorText = "";
}

void MprpcController::Reset()
{
    m_failed = false;
    m_errorText = "";
}

bool MprpcController::Failed() const
{
    return m_failed;
}

std::string MprpcController::ErrorText() const
{
    return m_errorText;
}

void MprpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errorText = reason;
}

// 目前未实现具体的功能
void MprpcController::StartCancel()
{

}

bool MprpcController::IsCanceled() const
{
    return false;
}

void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback)
{

}