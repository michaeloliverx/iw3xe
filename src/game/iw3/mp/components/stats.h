#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class stats : public Module
{
  public:
    stats();
    ~stats();

    static void UnlockEverything(unsigned int controllerIndex);
};
} // namespace mp
} // namespace iw3
