#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class ui_feeder : public Module
{
  public:
    ui_feeder();
    ~ui_feeder();

    static void OnDvarInit();
};
} // namespace mp
} // namespace iw3
