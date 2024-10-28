#pragma once

#include "Nous/Core/Buffer.h"

namespace Nous
{
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};
}
