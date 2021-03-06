#include "WorldGenMountains.h"
#include "Core\MapGen\PerlinNoise.h"
#include "Core\World.h"

#define GEN_OCT 2
float flatness(float tx, float ty)
{
	return (PerlinNoise2D(tx, ty, 2, 2, GEN_OCT) + 1) / 2.f;
}

float dens(float tx, float ty, float tz)
{
	float flat = 1 / 5.f;// flatness(tx / 1000.f, ty / 1000.f);
	if (tz < -SECTOR_SIZE)
		return 1;
	else if (tz < 0)
		return PerlinNoise3D(tx / 100.f, ty / 100.f, tz / 100.f, 1 + 5 * flat, 2, GEN_OCT) + ((-tz*(SECTOR_SIZE / 1000.f)));
	else
		return PerlinNoise3D(tx / 100.f, ty / 100.f, tz / 100.f, 1 + 5 * flat, 2, GEN_OCT) / ((tz + float(SECTOR_SIZE)) / float(SECTOR_SIZE));
}

float cluster(float tx, float ty, float tz)
{
	return PerlinNoise3D(tx / 3.f, ty / 3.f, tz / 3.f, 1 + 5, 2, 1);
}

// examples:
// 0.2 = nothing
// 0.3 = diamonds
// 0.31 = coal
// 0.4 = almost everything
// 1 = everything
bool is_cluster(float tx, float ty, float tz, float type, float prob)
{
	return cluster(tx + type * 3571, ty + type * 3557, tz + type * 3559) + 1 < prob * 2.f;
}

bool solid(float tx, float ty, float tz)
{
	return dens(tx, ty, tz) > 0.1;
}
#undef GEN_OCT

static std::vector<glm::vec3> treepos;
static std::vector<glm::vec3> bushpos;

void WorldGenMountains::WorldPass(World &w)
{
	if (treepos.empty())
		return;

	auto log = DB::Get().Create(StringIntern("log_block"));
	auto leaves = DB::Get().Create(StringIntern("leaves"));
	auto berry_bush = DB::Get().Create(StringIntern("berry_bush"));

	for (auto &tp : treepos)
	{
		int h = rand() % 10;

		for(int i = -3; i < 3; i++)
			for (int j = -3; j < 3; j++)
				for (int k = -3; k < 3; k++)
			{
				if(glm::length(glm::vec3(i,j,k)) <= 3)
				{
					w.SetBlock(tp + glm::vec3(0 + i, 0 + j, h + k), leaves->Clone(), true);
				}
			}

		for (int i = 0; i < h; i++)
		{
			w.SetBlock(tp + glm::vec3(0, 0, i), log->Clone(), true);
		}
	}

	for (auto &tp : bushpos)
	{
		int h = rand() % 3;
		for (int i = -3; i < 3; i++)
			for (int j = -3; j < 3; j++)
			{
				w.SetBlock(tp + glm::vec3(0 + i, 0 + j, 0), berry_bush->Clone(), true);
			}
	}

	treepos.clear();
	bushpos.clear();
	EventBus::Get().Publish<EventMapReady>();
}

void WorldGenMountains::Generate(Sector & s)
{
	const size_t size = static_cast<size_t>(SECTOR_SIZE);
	auto spos = s.GetPos();

	auto bg = DB::Get().Create(StringIntern("grass"));

	auto bd4 = DB::Get().Create(StringIntern("dirt4"));
	auto bd3 = DB::Get().Create(StringIntern("conglomerate"));
	auto bd2 = DB::Get().Create(StringIntern("dirt2"));
	auto bd = DB::Get().Create(StringIntern("dirt"));

	auto bf = DB::Get().Create(StringIntern("grass_high"));
	auto bf2 = DB::Get().Create(StringIntern("grass_high_small"));
	auto bf3 = DB::Get().Create(StringIntern("grass_high_small_white"));
	auto bf4 = DB::Get().Create(StringIntern("grass_high_small_yellow"));
	auto bf5 = DB::Get().Create(StringIntern("grass_high_small_blue"));
	auto bf6 = DB::Get().Create(StringIntern("grass_high_small_red"));
	auto bfb = DB::Get().Create(StringIntern("grass_high_small_bad"));

	auto ore_dot = DB::Get().Taglist("ore_dots");
	std::vector<PGameObject> ore_dots;
	for (const auto &r : ore_dot)
	{
		ore_dots.push_back(DB::Get().Create(r));
	}

	auto place_tree = [&](const glm::vec3 &pos) {
		if(rand()%8 == 1)
			treepos.push_back(pos);
	};

	auto place_berry = [&](const glm::vec3 &pos) {
		if (rand() % 8 == 1)
			bushpos.push_back(pos);
	};

	//auto rock = DB::Get().Taglist("rock");
	//std::vector<PGameObject> rocks;
	//for (const auto &r : rock)
	//{
	//	rocks.push_back(DB::Get().Create(r));
	//}

	for (int i = 0; i < SECTOR_SIZE; ++i)
	{
		for (int j = 0; j < SECTOR_SIZE; ++j)
		{
			for (int k = 0; k < SECTOR_SIZE; ++k)
			{
				float tx = static_cast<float>(i + spos.x*SECTOR_SIZE);
				float ty = static_cast<float>(j + spos.y*SECTOR_SIZE);
				float tz = static_cast<float>(k + spos.z*SECTOR_SIZE);

				if (solid(tx, ty, tz))
				{
					if (solid(tx, ty, tz + 15))
					{
						{
							bool some = false;
							for (int l = 0; l < ore_dots.size(); ++l)
							{
								if (is_cluster(tx, ty, tz, 5 + l, 0.3))
								{
									s.SetBlock({ i, j, k }, ore_dots[l]->Clone());
									some = true;
									break;
								}
							}
							if (!some)
								s.SetBlock({ i, j, k }, bd4);
						}
					}
					else if (solid(tx, ty, tz + 10))
						s.SetBlock({ i, j, k }, bd3);
					else if (solid(tx, ty, tz + 5))
						s.SetBlock({ i, j, k }, bd2);
					else if (!solid(tx, ty, tz + 1))
						s.SetBlock({ i, j, k }, bg);
					else
						s.SetBlock({ i, j, k }, bd);
				}
				else
				{
					if (solid(tx, ty, tz - 1))
					{
						if (rand() % 10 == 1)
							s.SetBlock({ i, j, k }, bf);
						else
							if (rand() % 4 == 1)
							{
								switch (int t = rand() % 10)
								{
								case 0:
									s.SetBlock({ i, j, k }, bf3);
									break;
								case 1:
									s.SetBlock({ i, j, k }, bf4);
									break;
								case 2:
									s.SetBlock({ i, j, k }, bf5);
									break;
								case 3:
									s.SetBlock({ i, j, k }, bf6);
									break;
								case 4:
									place_tree({ tx, ty, tz });
									break;
								case 5:
									place_berry({ tx, ty, tz });
									break;
								default:
									s.SetBlock({ i, j, k }, bf2);
								}
							}
					}
				}
			}
		}
	}
}
