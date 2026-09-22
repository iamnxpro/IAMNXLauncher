// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QtGlobal>

namespace WindowsGameTuning {
//! Applies reversible, process-scoped tuning. No Defender, firewall, registry, or injection changes are made.
void apply(qint64 processId);
}
