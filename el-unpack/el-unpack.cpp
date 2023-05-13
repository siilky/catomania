#include <iostream>
#include <system_error>

#include <boost/nowide/args.hpp>
#include <CLI11/CLI11.hpp>
#include <game/data/filereader.h>
#include <netio/mppc.h>


class FileWriter
{
    FileWriter(const FileWriter &) = delete;
    FileWriter operator=(const FileWriter &) = delete;
public:
    FileWriter() = default;
    ~FileWriter()
    {
        close();
    }

    bool open(const std::wstring & filename)
    {
        if (file_ != INVALID_HANDLE_VALUE)
        {
            close();
        }
        isGood_ = false;
        if (filename.empty())
        {
            return isGood_;
        }
        file_ = CreateFileW(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);
        if (file_ != INVALID_HANDLE_VALUE)
        {
            isGood_ = true;
        }
        return isGood_;
    }

    void close()
    {
        if (file_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(file_);
            file_ = INVALID_HANDLE_VALUE;
        }
        isGood_ = false;
    }

    bool write(const void * data, size_t size) const
    {
        if (file_ == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        DWORD written = 0;
        if (!WriteFile(file_, data, size, &written, 0))
        {
            throw std::system_error(GetLastError(), std::system_category());
        }
        if (written != size)
        {
            throw std::runtime_error(std::string("Partial file write: ") + std::to_string(written) + " of " + std::to_string(size));
        }
        return true;
    }

    bool write(const std::vector<uint8_t> & data) const
    {
        return write(data.data(), data.size());
    }
    bool write(const std::string & data) const
    {
        return write(data.data(), data.size());
    }
    bool write(const std::wstring & data) const
    {
        return write(data.data(), data.size() * sizeof(wchar_t));
    }
    bool write(const uint16_t & data) const
    {
        return write(&data, sizeof(uint16_t));
    }
    bool write(const uint32_t & data) const
    {
        return write(&data, sizeof(uint32_t));
    }
    bool write(const uint64_t & data) const
    {
        return write(&data, sizeof(uint64_t));
    }
    bool write(const int8_t & data) const
    {
        return write(&data, sizeof(int8_t));
    }
    bool write(const int16_t & data) const
    {
        return write(&data, sizeof(int16_t));
    }
    bool write(const int32_t & data) const
    {
        return write(&data, sizeof(int32_t));
    }
    bool write(const int64_t & data) const
    {
        return write(&data, sizeof(int64_t));
    }
    bool good() const
    {
        return isGood_;
    }
    bool is_open() const
    {
        return file_ != INVALID_HANDLE_VALUE;
    }

private:
    HANDLE file_ = INVALID_HANDLE_VALUE;
    bool isGood_ = false;
};


static int main_fn(int argc, char ** argv)
{
    CLI::App app{ "el-unpack" };
    app.allow_windows_style_options();

    std::string inputFile, outputFile;
    auto i1 = app.add_option("inputFile", inputFile, "Input file")->required()->check(CLI::ExistingFile);
    app.add_option("-o, --output", outputFile, "Output file");

    boost::nowide::args __(argc, argv);
    CLI11_PARSE(app, argc, argv);


    auto inputPath = std::filesystem::path(inputFile);
    auto outputPath = std::filesystem::path(outputFile);
    if (outputPath.empty())
    {
        outputPath = std::filesystem::current_path() / inputPath.filename();
        outputPath += ".out";
    }


    std::cout << "Input: " << inputFile << std::endl;
    FileReader fr;
    if (!fr.open(inputPath.wstring()))
    {
        std::cerr << "Failed to open " << inputPath << std::endl;
        return 1;
    }

    const auto elVersion = fr.readWord();
    const auto elSign = fr.readWord();
    const auto elTs = fr.readDword();

    std::cout << "EL: version " << elVersion << " TS " << elTs << std::endl;

    if (elVersion < 352)
    {
        std::cerr << "EL does not require processing" << std::endl;
        return 1;
    }


    auto list0s = fr.readDword();
    //fr.move(s1);
    auto list0 = fr.readBytes(list0s);

    //auto dec = std::make_shared<MPPCDecoder>();
    //auto chunk = dec->transform(list0);

    auto list21pd0 = fr.readDword();
    auto list21ps = fr.readDword();
    auto list21p = fr.readBytes(list21ps);
    auto list21pd1 = fr.readDword();

    auto list101pd0 = fr.readDword();
    auto list101pds = fr.readDword();
    auto list101p = fr.readBytes(list101pds);

    FileWriter fw;
    if (!fw.open(outputPath))
    {
        std::cerr << "Failed to open " << outputPath << " : " << GetLastError() << std::endl;
        return 1;
    }

    fw.write(elVersion);
    fw.write(elSign);
    fw.write(elTs);

    fw.write(list0.size());
    fw.write(list0);

    struct Entry
    {
        int32_t id;
        int16_t size;
    };

    int listId = 1;

    while (fr && !fr.eof())
    {
        if (listId == 21)
        {
            fw.write(list21pd0);
            fw.write(list21p.size());
            fw.write(list21p);
            fw.write(list21pd1);
        }
        else if (listId == 101)
        {
            fw.write(list101pd0);
            fw.write(list101p.size());
            fw.write(list101p);
        }

        std::vector<Entry> dir;

        auto itemsCount = fr.readDword();
        if (itemsCount > 0)
        {
            for (size_t i = 0; i < itemsCount; i++)
            {
                Entry e;
                e.id = fr.readDword();
                e.size = fr.readWord();
                dir.push_back(e);
            }

            int packedSize = fr.readDword();

            int unpackedChunkSize = 0;
            for (const auto & e : dir)
            {
                auto packedChunk = fr.readBytes(e.size);

                packedSize -= e.size;
                if (packedSize < 0)
                {
                    std::cerr << "List " << listId << " packed data exhausted: " << packedSize << std::endl;
                    return 1;
                }

                auto dec = std::make_unique<MPPCDecoder>();
                auto chunk = dec->transform(packedChunk);

                if (unpackedChunkSize == 0)
                {
                    // first entry
                    unpackedChunkSize = chunk.size();

                    fw.write(listId);
                    fw.write(itemsCount);
                    fw.write(unpackedChunkSize);
                }
                else if (unpackedChunkSize != chunk.size())
                {
                    std::cerr << "List " << listId << " chunk size mismatch : " << unpackedChunkSize << " != " << chunk.size() << std::endl;
                    return 1;
                }

                fw.write(chunk);
            }
        }
        else
        {
            fw.write(listId);
            fw.write(itemsCount);
            fw.write(0);    // dont know chunk size
        }

        listId++;
    }

    std::cout << "Done " << listId << " lists" << std::endl;

    return 0;
}


int main(int argc, char ** argv)
{
    try
    {
        main_fn(argc, argv);
    }
    catch (const std::system_error & se)
    {
        std::cerr << "System error: " << se.code() << " - " << se.what() << std::endl;
    }
    catch (const std::runtime_error & re)
    {
        std::cerr << "Runtime error: " << re.what() << std::endl;
    }
    catch (const std::exception & e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}