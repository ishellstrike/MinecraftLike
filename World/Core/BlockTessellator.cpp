#include "BlockTessellator.h"
#include "Sector.h"
#include <Serealize.h>

//possibly must be moved into <renderstrats> file
//deserialize autoreg
#include "SplitBlockTessellator.h"
#include "ModelTessellator.h"
#include "SplitConnectionTesselator.h"

void BlockTessellator::JsonLoad(const rapidjson::Value & val)
{
	JSONLOAD(sge::make_nvp("transparent", mTransparent));
}

BlockTessellatorFactory::FactoryType & BlockTessellatorFactory::Get()
{
	static FactoryType object;
	return object;
}
