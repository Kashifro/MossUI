#pragma once

#include <chrono>
#include <string>

namespace MossUI {

void ShowToast(std::string message, std::chrono::milliseconds duration = std::chrono::milliseconds(3000));

}
