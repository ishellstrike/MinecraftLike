#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"
#include "WorldWorker.h"
#include <thread>
#include "tools\CoordSystem.h"
#include "Core\Sector.h"
#include <gui\WindowPerfomance.h>
#include "WorldGenMountains.h"
#include <core/World.h>

struct membuf : std::streambuf
{
	membuf(char* begin, char* end) {
		this->setg(begin, begin, end);
	}
};


WorldWorker::WorldWorker(std::shared_ptr<World> w)
{
	this->w = w;
	mGenerator = std::make_unique<WorldGenMountains>();

	mz_zip_archive zip_archive;
	mz_bool status;

	memset(&zip_archive, 0, sizeof(zip_archive));

	status = mz_zip_reader_init_file(&zip_archive, Settings::Get().save_file.c_str(), 0);

	if (!status)
	{
		LOG(error) << "zip file appears invalid...";
		return;
	}

	for (int i = 0; i < static_cast<int>(mz_zip_reader_get_num_files(&zip_archive)); i++)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
		{
			LOG(error) << "zip file read error...";
			mz_zip_reader_end(&zip_archive);
			return;
		}

		size_t uncompressed_size = file_stat.m_uncomp_size;
		char* p = reinterpret_cast<char *>(mz_zip_reader_extract_file_to_heap(&zip_archive, file_stat.m_filename, &uncompressed_size, 0));
		if (!p)
		{
			LOG(error) << "mz_zip_reader_extract_file_to_heap() failed...";
			mz_zip_reader_end(&zip_archive);
			return;
		}

		membuf sbuf(p, p + uncompressed_size);
		std::istream is(&sbuf);

		if (strstr(file_stat.m_filename, ".sec") != nullptr)
		{
			auto s = std::make_shared<Sector>(SPos(0, 0, 0));

			try
			{
				s->BinLoad(is);

				mReady[s->GetPos()] = s;
			}
			catch (...)
			{
				LOG(error) << "broken sector " << file_stat.m_filename;
			}
		}

		if (strstr(file_stat.m_filename, ".rooms") != nullptr)
		{
			w->BinLoadRooms(is);
		}

		mz_free(p);
	}

	mz_zip_reader_end(&zip_archive);
}

WorldWorker::~WorldWorker()
{
}

void WorldWorker::SaveAll()
{
	remove(Settings::Get().save_file.c_str());

	for (const auto &s : mReady)
	{
		auto a_name = (boost::format("%1%_%2%_%3%.sec") % s.first.x % s.first.y % s.first.z).str();

		std::stringbuf str;
		std::ostream os(nullptr);
		os.rdbuf(&str);

		//if (os.is_open())
		{
			s.second->BinSave(os);

			mz_zip_add_mem_to_archive_file_in_place(Settings::Get().save_file.c_str(), a_name.c_str(), str.str().c_str(), str.str().size(), nullptr, 0, 0);
		}
	}

	std::stringbuf str;
	std::ostream os(nullptr);
	os.rdbuf(&str);

	w->BinSaveRooms(os);

	mz_zip_add_mem_to_archive_file_in_place(Settings::Get().save_file.c_str(), "rooms.rooms", str.str().c_str(), str.str().size(), nullptr, 0, 0);

}

std::shared_ptr<Sector> WorldWorker::GetSector(const SPos &v)
{
	std::lock_guard<std::mutex> scope(mQueueMutex);
	mLast = v;

	auto f = mReady.find(v);
	if (f != mReady.end())
		return f->second;

	if (mRequested.find(v) == mRequested.end())
		mRequested.insert(v);

	return nullptr;
}

void WorldWorker::Process()
{
	auto start = glfwGetTime();
	mQueueMutex.lock();
	if (!mRequested.empty())
	{
		SPos last = mLast;
		auto r = mRequested.find(last);
		if (r == mRequested.end())
			r = mRequested.begin();
		mQueueMutex.unlock();

		auto gen = Generate(*r);

		mQueueMutex.lock();
		mReady[*r] = gen;
		mRequested.erase(r);
		mQueueMutex.unlock();
	}
	else
	{
		WorldPass();
		mQueueMutex.unlock();
	}

	auto end = glfwGetTime();
	WindowPerfomance::Get().GeneratorDt(end - start);
}

void WorldWorker::WorldPass() const
{
	mGenerator->WorldPass(*w);
}

std::shared_ptr<Sector> WorldWorker::Generate(const SPos & spos)
{
	std::shared_ptr<Sector> psec = std::make_shared<Sector>(spos);
	Sector &sector = *psec;

	mGenerator->Generate(sector);

	return psec;
}

