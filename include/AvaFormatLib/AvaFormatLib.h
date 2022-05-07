#pragma once

#define AVAFORMATLIB_VERSION_MAJOR 1
#define AVAFORMATLIB_VERSION_MINOR 0
#define AVAFORMATLIB_VERSION_REVISION 0

#include "types.h"

#include "util/hashlittle.h"
#include "util/math.h"
#include "util/murmur3.h"

#include "archives/archive_table.h"
#include "archives/avalanche_archive_format.h"
#include "archives/oodle_helper.h"
#include "archives/resource_bundle.h"
#include "archives/stream_archive.h"

#include "models/avalanche_model_format.h"
#include "models/render_block_model.h"

#include "avalanche_data_format.h"
#include "avalanche_texture.h"
#include "runtime_property_container.h"
#include "shader_bundle.h"
#include "string_lookup.h"