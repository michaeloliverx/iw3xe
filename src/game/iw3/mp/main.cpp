#include "main.h"
#include "components/cg.h"
#include "components/cj_tas.h"
#include "components/clipmap.h"
#include "components/cmds.h"
#include "components/g_client_fields.h"
#include "components/pm.h"
#include "components/scr_vm_functions.h"

// Structure to hold data for the active keyboard request
struct KeyboardRequest
{
    wchar_t *resultText;
    XOVERLAPPED overlapped;
    std::function<void(bool, const wchar_t *)> callback;
    DWORD maxLength;
    bool isActive;

    KeyboardRequest() : resultText(nullptr), callback(nullptr), maxLength(0), isActive(false)
    {
        ZeroMemory(&overlapped, sizeof(XOVERLAPPED));
    }

    ~KeyboardRequest()
    {
        if (resultText)
        {
            delete[] resultText;
            resultText = nullptr;
        }
    }
};

// Global singleton for the keyboard request
static KeyboardRequest g_keyboardRequest;

// Function to show keyboard UI without blocking
bool ShowKeyboardAsync(
    DWORD dwUserIndex,
    const wchar_t *defaultText,
    const wchar_t *titleText,
    const wchar_t *descriptionText,
    DWORD maxTextLength,
    DWORD keyboardType,
    std::function<void(bool, const wchar_t *)> callback)
{
    // If there's already an active keyboard request, don't allow another one
    if (g_keyboardRequest.isActive)
    {
        DbgPrint("Keyboard UI is already active, ignoring request\n");
        return false;
    }

    // Clean up any previous request
    if (g_keyboardRequest.resultText)
    {
        delete[] g_keyboardRequest.resultText;
        g_keyboardRequest.resultText = nullptr;
    }

    // Allocate new result text buffer (add 1 for null terminator)
    DWORD bufferSize = maxTextLength + 1;
    g_keyboardRequest.resultText = new wchar_t[bufferSize];
    ZeroMemory(g_keyboardRequest.resultText, bufferSize * sizeof(wchar_t));
    g_keyboardRequest.maxLength = bufferSize;
    g_keyboardRequest.callback = callback;

    // Zero out the overlapped structure
    ZeroMemory(&g_keyboardRequest.overlapped, sizeof(XOVERLAPPED));

    // Set up the keyboard UI with overlapped to make it non-blocking
    HRESULT hr = XShowKeyboardUI(
        dwUserIndex,
        keyboardType,
        defaultText,
        titleText,
        descriptionText,
        g_keyboardRequest.resultText,
        bufferSize,
        &g_keyboardRequest.overlapped);

    if (FAILED(hr))
    {
        // Handle error
        DbgPrint("Failed to show keyboard UI: 0x%08X\n", hr);
        delete[] g_keyboardRequest.resultText;
        g_keyboardRequest.resultText = nullptr;
        return false;
    }

    // Mark as active
    g_keyboardRequest.isActive = true;
    DbgPrint("Keyboard UI requested, continuing with game loop...\n");
    return true;
}

// Function to call in your game loop to check for keyboard completion
void CheckKeyboardCompletion()
{
    // If no active keyboard request, nothing to do
    if (!g_keyboardRequest.isActive)
    {
        return;
    }

    // Check if the operation has completed
    if (XHasOverlappedIoCompleted(&g_keyboardRequest.overlapped))
    {
        DWORD result;
        XGetOverlappedResult(&g_keyboardRequest.overlapped, &result, TRUE);

        DbgPrint("Keyboard operation completed with result: 0x%08X\n", result);

        // Operation completed
        bool success = (result == ERROR_SUCCESS);

        // Call the user callback with the result
        if (g_keyboardRequest.callback)
        {
            if (success)
            {
                DbgPrint("Keyboard text entry successful\n");
                g_keyboardRequest.callback(true, g_keyboardRequest.resultText);
            }
            else
            {
                DbgPrint("Keyboard operation failed or canceled\n");
                g_keyboardRequest.callback(false, nullptr);
            }
        }

        // Clean up
        delete[] g_keyboardRequest.resultText;
        g_keyboardRequest.resultText = nullptr;
        g_keyboardRequest.isActive = false;
    }
    // If not completed, we'll check again next frame
}

void GPUEndianSwapTexture(std::vector<uint8_t> &pixelData, GPUENDIAN endianType)
{
    switch (endianType)
    {
    case GPUENDIAN_8IN16:
        XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_8IN16, 2, pixelData.size() / 2);
        break;
    case GPUENDIAN_8IN32:
        XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_8IN32, 4, pixelData.size() / 4);
        break;
    case GPUENDIAN_16IN32:
        XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_16IN32, 4, pixelData.size() / 4);
        break;
    }
}

// TODO: MAKEFOURCC('D', 'X', 'T', '1');
// DDS Constants
const uint32_t DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32_t DDS_HEADER_SIZE = 124;
const uint32_t DDS_PIXEL_FORMAT_SIZE = 32;
const uint32_t DDSD_CAPS = 0x1;
const uint32_t DDSD_HEIGHT = 0x2;
const uint32_t DDSD_WIDTH = 0x4;
const uint32_t DDSD_PIXELFORMAT = 0x1000;
const uint32_t DDSD_LINEARSIZE = 0x80000;
const uint32_t DDPF_FOURCC = 0x4;
const uint32_t DDPF_RGB = 0x40;
const uint32_t DDPF_ALPHAPIXELS = 0x1;
const uint32_t DDSCAPS_TEXTURE = 0x1000;
const uint32_t DDSCAPS_MIPMAP = 0x400000;
const uint32_t DDPF_LUMINANCE = 0x20000;

// DDS Pixel Formats (FourCC Codes)
const uint32_t DXT1_FOURCC = MAKEFOURCC('D', 'X', 'T', '1');
const uint32_t DXT3_FOURCC = MAKEFOURCC('D', 'X', 'T', '3');
const uint32_t DXT5_FOURCC = MAKEFOURCC('D', 'X', 'T', '5');
const uint32_t DXN_FOURCC = MAKEFOURCC('A', 'T', 'I', '2'); // (DXN / BC5)

// Additional DDS Cubemap Flags
const uint32_t DDSCAPS2_CUBEMAP = 0x200;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;

// DDS Header Structure (with inline endian swapping)
struct DDSHeader
{
    uint32_t magic;
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    struct
    {
        uint32_t size;
        uint32_t flags;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
        uint32_t aBitMask;
    } pixelFormat;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
};

static_assert(sizeof(DDSHeader) == 128, "");

struct DDSImage
{
    DDSHeader header;
    std::vector<uint8_t> data;
};

// Function to swap all necessary fields from little-endian to big-endian
void SwapDDSHeaderEndian(DDSHeader &header)
{
    header.magic = _byteswap_ulong(header.magic);
    header.size = _byteswap_ulong(header.size);
    header.flags = _byteswap_ulong(header.flags);
    header.height = _byteswap_ulong(header.height);
    header.width = _byteswap_ulong(header.width);
    header.pitchOrLinearSize = _byteswap_ulong(header.pitchOrLinearSize);
    header.depth = _byteswap_ulong(header.depth);
    header.mipMapCount = _byteswap_ulong(header.mipMapCount);

    for (int i = 0; i < 11; i++)
        header.reserved1[i] = _byteswap_ulong(header.reserved1[i]);

    header.pixelFormat.size = _byteswap_ulong(header.pixelFormat.size);
    header.pixelFormat.flags = _byteswap_ulong(header.pixelFormat.flags);
    header.pixelFormat.fourCC = _byteswap_ulong(header.pixelFormat.fourCC);
    header.pixelFormat.rgbBitCount = _byteswap_ulong(header.pixelFormat.rgbBitCount);
    header.pixelFormat.rBitMask = _byteswap_ulong(header.pixelFormat.rBitMask);
    header.pixelFormat.gBitMask = _byteswap_ulong(header.pixelFormat.gBitMask);
    header.pixelFormat.bBitMask = _byteswap_ulong(header.pixelFormat.bBitMask);
    header.pixelFormat.aBitMask = _byteswap_ulong(header.pixelFormat.aBitMask);

    header.caps = _byteswap_ulong(header.caps);
    header.caps2 = _byteswap_ulong(header.caps2);
    header.caps3 = _byteswap_ulong(header.caps3);
    header.caps4 = _byteswap_ulong(header.caps4);
    header.reserved2 = _byteswap_ulong(header.reserved2);
}

DDSImage ReadDDSFile(const std::string &filepath)
{
    DDSImage ddsImage;
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open())
    {
        DbgPrint("ERROR: Unable to open file: %s\n", filepath.c_str());
        return ddsImage; // Return empty DDSImage
    }

    // Read DDS header (raw, little-endian)
    file.read(reinterpret_cast<char *>(&ddsImage.header), sizeof(DDSHeader));

    // Swap only the magic number to big-endian for proper validation
    uint32_t magicSwapped = _byteswap_ulong(ddsImage.header.magic);

    if (magicSwapped != 0x20534444) // 'DDS ' in big-endian
    {
        DbgPrint("ERROR: Invalid DDS file: %s\n", filepath.c_str());
        file.close();
        return ddsImage;
    }

    // Swap header fields to big-endian for Xbox 360
    SwapDDSHeaderEndian(ddsImage.header);

    // Move to end of file to get total file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();

    // Ensure fileSize is valid before proceeding
    if (fileSize == std::streampos(-1))
    {
        DbgPrint("ERROR: Failed to determine file size.\n");
        file.close();
        return ddsImage;
    }

    // Move back to after the header
    file.seekg(sizeof(DDSHeader), std::ios::beg);

    // Compute data size safely
    size_t dataSize = static_cast<size_t>(fileSize) - sizeof(DDSHeader);

    // Read image data
    ddsImage.data.resize(dataSize);
    file.read(reinterpret_cast<char *>(ddsImage.data.data()), dataSize);

    file.close();

    // Debug output
    DbgPrint("INFO: DDS file '%s' loaded successfully.\n", filepath.c_str());
    DbgPrint("      Resolution: %ux%u\n", ddsImage.header.width, ddsImage.header.height);
    DbgPrint("      MipMaps: %u\n", ddsImage.header.mipMapCount);
    DbgPrint("      Data Size: %u bytes\n", static_cast<unsigned int>(dataSize));

    return ddsImage;
}

namespace iw3
{
    namespace mp
    {

        Detour CL_ConsolePrint_Detour;

        void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags)
        {
            CL_ConsolePrint_Detour.GetOriginal<decltype(CL_ConsolePrint)>()(localClientNum, channel, txt, duration, pixelWidth, flags);
            DbgPrint("CL_ConsolePrint txt=%s \n", txt);
        }

        void Cmd_cmdinput_f()
        {
            bool success = ShowKeyboardAsync(
                0,                         // First controller
                L"",                       // Default text
                L"Enter Text",             // Title
                L"Please enter some text", // Description
                256,                       // Max length
                VKBD_DEFAULT,              // Keyboard type
                [](bool success, const wchar_t *text)
                {
                    if (success && text)
                    {
                        // Get the required buffer size
                        size_t wideLength = wcslen(text);
                        size_t mbBufferSize = wideLength * 4 + 1; // 4 bytes per character worst case

                        // Create buffer and convert
                        std::vector<char> mbBuffer(mbBufferSize);
                        wcstombs_s(nullptr, mbBuffer.data(), mbBufferSize, text, wideLength);

                        // Create string from the buffer
                        std::string result = mbBuffer.data();

                        Cbuf_AddText(0, result.c_str());
                    }
                    else
                    {
                        DbgPrint("Keyboard operation failed or was canceled\n");
                    }
                });

            if (!success)
            {
                DbgPrint("Failed to open keyboard UI\n");
            }
        }

        Detour CL_GamepadButtonEvent_Detour;

        void CL_GamepadButtonEvent_Hook(int localClientNum, int controllerIndex, int key, int down, unsigned int time)
        {
            CL_GamepadButtonEvent_Detour.GetOriginal<decltype(CL_GamepadButtonEvent)>()(localClientNum, controllerIndex, key, down, time);

            // Check if the client is disconnected (main menu)
            if (clientUIActives[localClientNum].connectionState == CA_DISCONNECTED)
            {
                if (key == K_BUTTON_RSTICK && down)
                {
                    Cmd_cmdinput_f();
                }
            }
        }

        Detour Load_MapEntsPtr_Detour;

        void Load_MapEntsPtr_Hook()
        {
            // TODO: don't write null byte to file
            // and add null byte to entityString when reading from file

            DbgPrint("Load_MapEntsPtr_Hook\n");

            // TODO: write comment what this is ***
            // Get pointer to pointer stored at 0x82475914
            MapEnts **varMapEntsPtr = *(MapEnts ***)0x82475914;

            Load_MapEntsPtr_Detour.GetOriginal<decltype(&Load_MapEntsPtr_Hook)>()();

            // Validate pointer before dereferencing
            if (varMapEntsPtr && *varMapEntsPtr)
            {
                MapEnts *mapEnts = *varMapEntsPtr;

                // Write stock map ents to disk
                std::string file_path = "game:\\dump\\";
                file_path += mapEnts->name;
                file_path += ".ents";                                        //  iw4x naming convention
                std::replace(file_path.begin(), file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
                filesystem::write_file_to_disk(file_path.c_str(), mapEnts->entityString, mapEnts->numEntityChars);

                // Load map ents from file
                // Path to check for existing entity file
                std::string raw_file_path = "game:\\raw\\";
                raw_file_path += mapEnts->name;
                raw_file_path += ".ents";                                            // IW4x naming convention
                std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes

                // If the file exists, replace entityString
                if (filesystem::file_exists(raw_file_path))
                {
                    DbgPrint("Found entity file: %s\n", raw_file_path.c_str());
                    std::string new_entity_string = filesystem::read_file_to_string(raw_file_path);
                    if (!new_entity_string.empty())
                    {
                        // Allocate new memory and copy the data
                        size_t new_size = new_entity_string.size() + 1; // Include null terminator
                        char *new_memory = static_cast<char *>(malloc(new_size));

                        if (new_memory)
                        {
                            memcpy(new_memory, new_entity_string.c_str(), new_size); // Copy with null terminator

                            // Update the entityString pointer to point to the new memory
                            mapEnts->entityString = new_memory;

                            // // Update numEntityChars
                            // mapEnts->numEntityChars = static_cast<int>(new_entity_string.size());	// unnecessary

                            DbgPrint("Replaced entityString from file: %s\n", raw_file_path.c_str());
                        }
                        else
                        {
                            DbgPrint("Failed to allocate memory for entityString replacement.\n");
                        }
                    }
                }
            }
            else
            {
                DbgPrint("Hooked Load_MapEntsPtr: varMapEntsPtr is NULL or invalid.\n");
            }
        }

        std::string extractFilename(const char *filename)
        {
            std::string path(filename);

            // Find last backslash '\' or forward slash '/'
            size_t lastSlash = path.find_last_of("\\/");
            size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;

            // Find last dot '.' (extension separator)
            size_t lastDot = path.find_last_of('.');
            size_t end = (lastDot == std::string::npos || lastDot < start) ? path.length() : lastDot;

            return path.substr(start, end - start);
        }

        bool R_StreamLoadHighMipReplacement(const char *filename, unsigned int bytesToRead, unsigned __int8 *outData)
        {
            std::string asset_name = extractFilename(filename);
            auto asset = DB_FindXAssetEntry(ASSET_TYPE_IMAGE, asset_name.c_str());

            if (!asset)
            {
                return false;
            }

            auto image = asset->entry.asset.header.image;

            std::string replacement_path = "game:\\raw\\highmip\\" + asset_name + ".dds";
            std::ifstream file(replacement_path, std::ios::binary | std::ios::ate);
            if (!file)
            {
                return false;
            }

            std::streamsize file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            if (file_size - 0x80 != bytesToRead) // 0x80 is the size of the DDS header
            {
                Com_PrintError(CON_CHANNEL_ERROR, "R_StreamLoadHighMipReplacement: File size mismatch: %s\n", replacement_path);
                return false;
            }

            std::vector<unsigned char> ddsHeader(0x80);
            file.read(reinterpret_cast<char *>(ddsHeader.data()), 0x80);

            // TODO: check if file is DDS and has correct format and dimensions

            std::vector<unsigned char> buffer;
            buffer.resize(static_cast<size_t>(bytesToRead));
            file.read(reinterpret_cast<char *>(buffer.data()), bytesToRead);

            GPUEndianSwapTexture(buffer, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

            XGTEXTURE_DESC textureDesc;
            XGGetTextureDesc(image->texture.basemap, 0, &textureDesc);

            // High mip are 2x the size of the original image
            auto width = image->width * 2;
            auto height = image->height * 2;
            auto rowPitch = textureDesc.RowPitch * 2;

            XGTileTextureLevel(
                width,
                height,
                0,
                image->texture.basemap->Format.DataFormat,
                XGTILE_NONPACKED, // Use non-packed mode (likely required for this texture)
                outData,          // Destination (tiled GPU memory for Base)
                nullptr,          // No offset (tile the whole image)
                buffer.data(),    // Source mip level data
                rowPitch,         // Row pitch of source image (should match DDS format)
                nullptr           // No subrectangle (tile the full image)
            );

            return true;
        }

        Detour R_StreamLoadFileSynchronously_Detour;

        int R_StreamLoadFileSynchronously_Hook(const char *filename, unsigned int bytesToRead, unsigned __int8 *outData)
        {
            if (R_StreamLoadHighMipReplacement(filename, bytesToRead, outData))
            {
                return 1;
            }

            // Fallback to original path if modified path failed
            return R_StreamLoadFileSynchronously_Detour.GetOriginal<decltype(R_StreamLoadFileSynchronously)>()(filename, bytesToRead, outData);
        }

        Detour Scr_ReadFile_FastFile_Detour;

        char *Scr_ReadFile_FastFile_Hook(const char *filename, const char *extFilename, const char *codePos, bool archive)
        {
            DbgPrint("Scr_ReadFile_FastFile_Hook extFilename=%s \n", extFilename);

            std::string raw_file_path = "game:\\raw\\";
            raw_file_path += extFilename;
            std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
            if (filesystem::file_exists(raw_file_path))
            {
                DbgPrint("Found raw file: %s\n", raw_file_path.c_str());
                // return ReadFileContents(raw_file_path);
                std::string new_contents = filesystem::read_file_to_string(raw_file_path);
                if (!new_contents.empty())
                {

                    // Allocate new memory and copy the data
                    size_t new_size = new_contents.size() + 1; // Include null terminator
                    char *new_memory = static_cast<char *>(malloc(new_size));

                    if (new_memory)
                    {
                        memcpy(new_memory, new_contents.c_str(), new_size); // Copy with null terminator

                        DbgPrint("Replaced contents from file: %s\n", raw_file_path.c_str());
                        return new_memory;
                    }
                    else
                    {
                        DbgPrint("Failed to allocate memory for contents replacement.\n");
                    }
                }
            }

            return Scr_ReadFile_FastFile_Detour.GetOriginal<decltype(&Scr_ReadFile_FastFile_Hook)>()(filename, extFilename, codePos, archive);
        }

        void Image_DbgPrint(const GfxImage *image)
        {
            const int format = image->texture.basemap->Format.DataFormat;
            char *format_str;
            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
                format_str = "DXT1";
                break;
            case GPUTEXTUREFORMAT_DXT2_3:
                format_str = "DXT2_3";
                break;
            case GPUTEXTUREFORMAT_DXT4_5:
                format_str = "DXT4_5";
                break;
            case GPUTEXTUREFORMAT_DXN:
                format_str = "DXN";
                break;
            case GPUTEXTUREFORMAT_8:
                format_str = "8";
                break;
            case GPUTEXTUREFORMAT_8_8:
                format_str = "8_8";
                break;
            case GPUTEXTUREFORMAT_8_8_8_8:
                format_str = "8_8_8_8";
                break;
            default:
                format_str = "UNKNOWN";
                break;
            }

            XGTEXTURE_DESC SourceDesc;
            XGGetTextureDesc(image->texture.basemap, 0, &SourceDesc);
            BOOL IsBorderTexture = XGIsBorderTexture(image->texture.basemap);
            UINT MipTailBaseLevel = XGGetMipTailBaseLevel(SourceDesc.Width, SourceDesc.Height, IsBorderTexture);

            // SourceDesc.BitsPerPixel;
            // SourceDesc.BytesPerBlock;

            UINT MipLevelCount = image->texture.basemap->GetLevelCount();

            UINT BaseSize;
            XGGetTextureLayout(image->texture.basemap, 0, &BaseSize, 0, 0, 0, 0, 0, 0, 0, 0);

            Com_Printf(CON_CHANNEL_CONSOLEONLY, "Image_DbgPrint: Dumping image Name='%s', Type=%d, Dimensions=%dx%d, MipLevels=%d, MipTailBaseLevel=%d, Format=%s, BitsPerPixel=%d, BytesPerBlock=%d, Endian=%d, BaseSize=%d\n",
                       image->name,
                       image->mapType,
                       image->width,
                       image->height,
                       MipLevelCount,
                       MipTailBaseLevel,
                       format_str,
                       SourceDesc.BitsPerPixel,
                       SourceDesc.BytesPerBlock,
                       image->texture.basemap->Format.Endian,
                       BaseSize);
        }

        void Image_Dump(const GfxImage *image)
        {
            // TODO: cleanup empty files if failed

            Com_Printf(CON_CHANNEL_CONSOLEONLY, "Image_Dump: Dumping image '%s'\n", image->name);

            if (!image)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Null GfxImage!\n");
                return;
            }

            if (!image->pixels || image->baseSize == 0)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Image '%s' has no valid pixel data!\n", image->name);
                return;
            }

            if (image->mapType != MAPTYPE_2D && image->mapType != MAPTYPE_CUBE)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported map type %d!\n", image->mapType);
                return;
            }

            UINT BaseSize;
            XGGetTextureLayout(image->texture.basemap, 0, &BaseSize, 0, 0, 0, 0, 0, 0, 0, 0);

            DDSHeader header;
            memset(&header, 0, sizeof(DDSHeader));

            header.magic = _byteswap_ulong(DDS_MAGIC);
            header.size = _byteswap_ulong(DDS_HEADER_SIZE);
            header.flags = _byteswap_ulong(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE);
            header.height = _byteswap_ulong(image->height);
            header.width = _byteswap_ulong(image->width);
            header.depth = _byteswap_ulong(image->depth);
            header.mipMapCount = _byteswap_ulong(image->texture.basemap->GetLevelCount());

            auto format = image->texture.basemap->Format.DataFormat;
            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
                header.pixelFormat.fourCC = _byteswap_ulong(DXT1_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_DXT2_3:
                header.pixelFormat.fourCC = _byteswap_ulong(DXT3_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_DXT4_5:
                header.pixelFormat.fourCC = _byteswap_ulong(DXT5_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_DXN:
                header.pixelFormat.fourCC = _byteswap_ulong(DXN_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_8:
                header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE);
                header.pixelFormat.rgbBitCount = _byteswap_ulong(8);
                header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
                header.pixelFormat.gBitMask = 0;
                header.pixelFormat.bBitMask = 0;
                header.pixelFormat.aBitMask = 0;
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_8_8:
                header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE | DDPF_ALPHAPIXELS);
                header.pixelFormat.rgbBitCount = _byteswap_ulong(16);
                header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
                header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
                header.pixelFormat.bBitMask = 0;
                header.pixelFormat.aBitMask = 0;
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_8_8_8_8:
                header.pixelFormat.flags = _byteswap_ulong(DDPF_RGB | DDPF_ALPHAPIXELS);
                header.pixelFormat.rgbBitCount = _byteswap_ulong(32);
                header.pixelFormat.rBitMask = _byteswap_ulong(0x00FF0000);
                header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
                header.pixelFormat.bBitMask = _byteswap_ulong(0x000000FF);
                header.pixelFormat.aBitMask = _byteswap_ulong(0xFF000000);
                header.pitchOrLinearSize = BaseSize;
                break;
            default:
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported texture format %d!\n", format);
                return;
            }

            // Set texture capabilities
            header.caps = _byteswap_ulong(DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);

            // Handle Cubemaps
            if (image->mapType == mp::MAPTYPE_CUBE)
            {
                header.caps2 = _byteswap_ulong(DDSCAPS2_CUBEMAP |
                                               DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX |
                                               DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY |
                                               DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ);
            }

            std::string filename = "game:\\dump\\images\\";
            std::string sanitized_name = image->name;

            // Remove invalid characters
            sanitized_name.erase(std::remove_if(sanitized_name.begin(), sanitized_name.end(), [](char c)
                                                { return c == '*'; }),
                                 sanitized_name.end());

            filename += sanitized_name + ".dds";

            std::ofstream file(filename, std::ios::binary);
            if (!file)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to open file: %s\n", filename.c_str());
                return;
            }

            if (image->mapType == MAPTYPE_CUBE)
            {
                file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

                unsigned int face_size = 0;
                unsigned int rowPitch = 0;
                const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);

                switch (format)
                {
                case GPUTEXTUREFORMAT_DXT1:
                    face_size = (image->width / 4) * (image->height / 4) * 8;
                    rowPitch = (image->width / 4) * 8; // 8 bytes per 4x4 block
                    break;
                case GPUTEXTUREFORMAT_8_8_8_8:
                    face_size = image->width * image->height * 4;
                    rowPitch = image->width * 4; // 4 bytes per pixel
                    break;
                default:
                    Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported cube map format %d!\n", format);
                    return;
                }

                // TODO: handle mip levels per face for cubemaps
                for (int i = 0; i < 6; i++)
                {
                    unsigned char *face_pixels = image->pixels + (i * face_size); // Offset for each face

                    std::vector<uint8_t> swappedFace(face_pixels, face_pixels + face_size);
                    GPUEndianSwapTexture(swappedFace, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                    // Create buffer for linear texture data
                    std::vector<uint8_t> linearFace(face_size);

                    // Convert tiled texture to linear layout using XGUntileTextureLevel
                    XGUntileTextureLevel(
                        image->width,               // Width
                        image->height,              // Height
                        0,                          // Level (base level)
                        static_cast<DWORD>(format), // GpuFormat
                        0,                          // Flags (no special flags)
                        linearFace.data(),          // pDestination (linear output)
                        rowPitch,                   // RowPitch
                        nullptr,                    // pPoint (no offset)
                        swappedFace.data(),         // pSource (tiled input)
                        nullptr                     // pRect (entire texture)
                    );

                    file.write(reinterpret_cast<const char *>(linearFace.data()), linearFace.size());
                }

                file.close();
            }
            else if (image->mapType == MAPTYPE_2D)
            {
                // TODO: write mip levels
                file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

                std::vector<uint8_t> pixelData(image->pixels, image->pixels + image->baseSize);

                GPUEndianSwapTexture(pixelData, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                // Create a linear data buffer to hold the untiled texture
                std::vector<uint8_t> linearData(image->baseSize);

                // Calculate row pitch based on format
                UINT rowPitch;
                auto format = image->texture.basemap->Format.DataFormat;

                switch (format)
                {
                case GPUTEXTUREFORMAT_DXT1:
                case GPUTEXTUREFORMAT_DXT2_3:
                case GPUTEXTUREFORMAT_DXT4_5:
                case GPUTEXTUREFORMAT_DXN:
                    // Block compressed formats use 4x4 blocks
                    rowPitch = ((image->width + 3) / 4) * (format == GPUTEXTUREFORMAT_DXT1 ? 8 : 16);
                    break;
                case GPUTEXTUREFORMAT_8:
                    rowPitch = image->width;
                    break;
                case GPUTEXTUREFORMAT_8_8:
                    rowPitch = image->width * 2;
                    break;
                case GPUTEXTUREFORMAT_8_8_8_8:
                    rowPitch = image->width * 4;
                    break;
                default:
                    Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported texture format %d!\n", format);
                    return;
                }

                DbgPrint("Image_Dump: rowPitch=%d\n", rowPitch);

                // Call XGUntileTextureLevel to convert the tiled texture to linear format
                XGUntileTextureLevel(
                    image->width,               // Width
                    image->height,              // Height
                    0,                          // Level (base level 0)
                    static_cast<DWORD>(format), // GpuFormat
                    XGTILE_NONPACKED,           // Flags (no special flags)
                    linearData.data(),          // pDestination
                    rowPitch,                   // RowPitch (calculated based on format)
                    nullptr,                    // pPoint (no offset)
                    pixelData.data(),           // pSource
                    nullptr                     // pRect (entire texture)
                );

                file.write(reinterpret_cast<const char *>(linearData.data()), linearData.size());

                file.close();
            }
            else
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported map type %d!\n", image->mapType);
                return;
            }
        }

        void Cmd_imagedump()
        {
            ImageList imageList;
            R_GetImageList(&imageList);

            // images bundled in xex
            // auto g_imageProgs = reinterpret_cast<GfxImage *>(0x84FEA6D0);
            // for (unsigned int i = 0; i < 10; i++)
            // {
            //     imageList.image[imageList.count++] = &g_imageProgs[i];
            // }

            Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumping %d images to 'raw\\images\\' %d\n", imageList.count);

            CreateDirectoryA("game:\\dump", 0);
            CreateDirectoryA("game:\\dump\\images", 0);
            CreateDirectoryA("game:\\dump\\highmip", 0);

            for (unsigned int i = 0; i < imageList.count; i++)
            {
                auto image = imageList.image[i];
                // DbgPrint(
                //     "Image %d: Name='%s', Type=%d, Dimensions=%dx%dx%d, MipLevels=%d, Format=%d, CardMemory=%d bytes, BaseSize=%d, Streaming=%d, DelayLoad=%d, Semantic=%d, StreamSlot=%d\n",
                //     i,
                //     image->name,
                //     image->mapType,
                //     image->width,
                //     image->height,
                //     image->depth, // Added depth for 3D textures
                //     image->texture.basemap->Format.MaxMipLevel + 1,
                //     image->texture.basemap->Format.DataFormat,
                //     image->cardMemory.platform[1], // Total memory allocation for the texture
                //     image->baseSize,               // Base memory size (for non-streaming textures)
                //     image->streaming,              // Whether the image is streamed
                //     image->delayLoadPixels,        // Whether pixel data is delayed in loading
                //     image->semantic,               // Purpose of the texture in the engine
                //     image->streamSlot              // Streaming slot ID
                // );
                Image_DbgPrint(image);

                Image_Dump(image);
            }

            auto highmips = filesystem::list_files_in_directory("D:\\highmip");
            for (size_t i = 0; i < highmips.size(); ++i)
            {
                const std::string &filepath = "D:\\highmip\\" + highmips[i];
                Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumping highmip file: %s\n", filepath.c_str());
                std::string assetName = extractFilename(filepath.c_str());
                auto asset = DB_FindXAssetEntry(ASSET_TYPE_IMAGE, assetName.c_str());
                if (!asset)
                {
                    Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' not found in asset list!\n", assetName.c_str());
                    continue;
                }

                auto image = asset->entry.asset.header.image;

                std::ifstream input_file(filepath, std::ios::binary | std::ios::ate); // Open file in binary mode and seek to end
                if (!input_file)
                {
                    Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to open file: %s\n", filepath.c_str());
                    continue;
                }

                std::streamsize size = input_file.tellg();
                if (size < 0)
                {
                    Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to determine file size: %s\n", filepath.c_str());
                    continue;
                }

                input_file.seekg(0, std::ios::beg);
                std::vector<uint8_t> buffer(static_cast<size_t>(size));

                if (input_file.read(reinterpret_cast<char *>(buffer.data()), size))
                {
                    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Read %d bytes from file.\n", size);
                }
                else
                {
                    Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Error reading file: %s\n", filepath.c_str());
                    continue;
                }

                auto width = image->width * 2;
                auto height = image->height * 2;
                auto baseSize = width * height * 4;

                DDSHeader header;
                memset(&header, 0, sizeof(DDSHeader));

                header.magic = _byteswap_ulong(DDS_MAGIC);
                header.size = _byteswap_ulong(DDS_HEADER_SIZE);
                header.flags = _byteswap_ulong(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE);
                header.width = _byteswap_ulong(width);
                header.height = _byteswap_ulong(height);
                header.depth = _byteswap_ulong(image->depth);
                header.mipMapCount = _byteswap_ulong(1);
                header.caps = _byteswap_ulong(DDSCAPS_TEXTURE);
                header.pitchOrLinearSize = baseSize;

                auto format = image->texture.basemap->Format.DataFormat;
                switch (format)
                {
                case GPUTEXTUREFORMAT_DXT1:
                    header.pixelFormat.fourCC = _byteswap_ulong(DXT1_FOURCC);

                    break;
                case GPUTEXTUREFORMAT_DXT2_3:
                    header.pixelFormat.fourCC = _byteswap_ulong(DXT3_FOURCC);

                    break;
                case GPUTEXTUREFORMAT_DXT4_5:
                    header.pixelFormat.fourCC = _byteswap_ulong(DXT5_FOURCC);

                    break;
                case GPUTEXTUREFORMAT_DXN:
                    header.pixelFormat.fourCC = _byteswap_ulong(DXN_FOURCC);

                    break;
                case GPUTEXTUREFORMAT_8:
                    header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE);
                    header.pixelFormat.rgbBitCount = _byteswap_ulong(8);
                    header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
                    header.pixelFormat.gBitMask = 0;
                    header.pixelFormat.bBitMask = 0;
                    header.pixelFormat.aBitMask = 0;

                    break;
                case GPUTEXTUREFORMAT_8_8:
                    header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE | DDPF_ALPHAPIXELS);
                    header.pixelFormat.rgbBitCount = _byteswap_ulong(16);
                    header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
                    header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
                    header.pixelFormat.bBitMask = 0;
                    header.pixelFormat.aBitMask = 0;

                    break;
                case GPUTEXTUREFORMAT_8_8_8_8:
                    header.pixelFormat.flags = _byteswap_ulong(DDPF_RGB | DDPF_ALPHAPIXELS);
                    header.pixelFormat.rgbBitCount = _byteswap_ulong(32);
                    header.pixelFormat.rBitMask = _byteswap_ulong(0x00FF0000);
                    header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
                    header.pixelFormat.bBitMask = _byteswap_ulong(0x000000FF);
                    header.pixelFormat.aBitMask = _byteswap_ulong(0xFF000000);

                    break;
                default:
                    Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported texture format %d!\n", format);
                    return;
                }

                // TODO: add sanity checks for format, size, etc.
                // TODO: handle filenames with unsupported characters for Windows

                auto output_filepath = "game:\\dump\\highmip\\" + assetName + ".dds";

                std::ofstream output_file(output_filepath, std::ios::binary);
                if (!output_file)
                {
                    Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to open file: %s\n", output_filepath.c_str());
                    return;
                }

                output_file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

                GPUEndianSwapTexture(buffer, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                // Calculate row pitch based on format
                UINT rowPitch;

                switch (format)
                {
                case GPUTEXTUREFORMAT_DXT1:
                    rowPitch = (width / 4) * 8; // 8 bytes per 4x4 block
                    break;
                case GPUTEXTUREFORMAT_DXT2_3:
                case GPUTEXTUREFORMAT_DXT4_5:
                case GPUTEXTUREFORMAT_DXN:
                    rowPitch = (width / 4) * 16; // 16 bytes per 4x4 block
                    break;
                case GPUTEXTUREFORMAT_8:
                    rowPitch = width; // 1 byte per pixel
                    break;
                case GPUTEXTUREFORMAT_8_8:
                    rowPitch = width * 2; // 2 bytes per pixel
                    break;
                case GPUTEXTUREFORMAT_8_8_8_8:
                    rowPitch = width * 4; // 4 bytes per pixel
                    break;
                default:
                    rowPitch = width * 4; // Default to 4 bytes per pixel
                    break;
                }

                // Create a buffer for linear texture data
                std::vector<uint8_t> linearData(buffer.size());
                std::vector<uint8_t> bufferAsUint8(buffer.begin(), buffer.end());

                // Convert tiled texture to linear layout
                XGUntileTextureLevel(
                    width,                      // Width
                    height,                     // Height
                    0,                          // Level (base level)
                    static_cast<DWORD>(format), // GpuFormat
                    0,                          // Flags (no special flags)
                    linearData.data(),          // pDestination (linear output)
                    rowPitch,                   // RowPitch
                    nullptr,                    // pPoint (no offset)
                    bufferAsUint8.data(),       // pSource (tiled input)
                    nullptr                     // pRect (entire texture)
                );

                output_file.write(reinterpret_cast<const char *>(linearData.data()), linearData.size());
                output_file.close();

                Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumped highmip file: %s\n", output_filepath.c_str());
            }
        }

        UINT CalculateMipLevelSize(UINT width, UINT height, UINT mipLevel, GPUTEXTUREFORMAT format)
        {
            // Calculate dimensions for the requested mip level
            UINT mipWidth = max(1, width >> mipLevel);
            UINT mipHeight = max(1, height >> mipLevel);

            // Calculate size based on format
            UINT blockSize;
            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
                blockSize = 8; // 8 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_DXT2_3:
                blockSize = 16; // 16 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_DXT4_5:
                blockSize = 16; // 16 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_DXN:
                blockSize = 16; // 16 bytes per 4x4 block (two 8-byte channels)
                break;
            default:
                DbgPrint("CalculateMipLevelSize: Unsupported format %d\n", format);
                return 0;
            }

            // For block-compressed formats, calculate number of blocks
            // Each block is 4x4 pixels, so we need to round up to nearest block
            UINT blocksWide = (mipWidth + 3) / 4;
            UINT blocksHigh = (mipHeight + 3) / 4;

            // Calculate total size in bytes
            UINT sizeInBytes = blocksWide * blocksHigh * blockSize;

            return sizeInBytes;
        }

        void Image_Replace_2D(GfxImage *image, const DDSImage &ddsImage)
        {
            if (image->mapType != MAPTYPE_2D)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a 2D map!\n", image->name);
                return;
            }

            // Get base texture layout
            UINT baseAddress, baseSize, mipAddress, mipSize;

            XGGetTextureLayout(image->texture.basemap,
                               &baseAddress, &baseSize, 0, 0, 0,
                               &mipAddress, &mipSize, 0, 0, 0);

            XGTEXTURE_DESC TextureDesc;
            XGGetTextureDesc(image->texture.basemap, 0, &TextureDesc);

            UINT mipTailBaseLevel = XGGetMipTailBaseLevel(TextureDesc.Width, TextureDesc.Height, XGIsBorderTexture(image->texture.basemap));

            UINT ddsOffset = 0;

            for (UINT mipLevel = 0; mipLevel < mipTailBaseLevel; mipLevel++)
            {
                UINT widthInBlocks = max(1, TextureDesc.WidthInBlocks >> mipLevel);
                UINT rowPitch = widthInBlocks * TextureDesc.BytesPerBlock;
                // UINT levelSize = rowPitch * heightInBlocks;
                UINT ddsMipLevelSize = CalculateMipLevelSize(image->width, image->height, mipLevel, static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat));

                if (ddsMipLevelSize == 0)
                {
                    DbgPrint("  [ERROR] Unsupported format %d for mip level %u! Skipping...\n", image->texture.basemap->Format.DataFormat, mipLevel);
                    break;
                }

                // Ensure we're not reading out of bounds
                if (ddsOffset + ddsMipLevelSize > ddsImage.data.size())
                {
                    DbgPrint("  [ERROR] Mip Level %u exceeds DDS data size! Skipping...\n", mipLevel);
                    break;
                }

                std::vector<uint8_t> levelData(ddsImage.data.begin() + ddsOffset, ddsImage.data.begin() + ddsOffset + ddsMipLevelSize);

                GPUEndianSwapTexture(levelData, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                DbgPrint("Image_Replace_2D: Mip Level %d - Row Pitch=%u\n", mipLevel, rowPitch);

                UINT address = baseAddress;
                if (mipLevel > 0)
                {
                    UINT mipLevelOffset = XGGetMipLevelOffset(image->texture.basemap, 0, mipLevel);
                    address = mipAddress + mipLevelOffset;
                }

                DbgPrint("Image_Replace_2D: Writing mip level %d to address 0x%08X - levelSize=%u\n", mipLevel, address, ddsMipLevelSize);

                // // Write the base level
                XGTileTextureLevel(
                    TextureDesc.Width,
                    TextureDesc.Height,
                    mipLevel,
                    image->texture.basemap->Format.DataFormat,
                    XGTILE_NONPACKED,                  // Use non-packed mode (likely required for this texture)
                    reinterpret_cast<void *>(address), // Destination (tiled GPU memory for Base)
                    nullptr,                           // No offset (tile the whole image)
                    levelData.data(),                  // Source mip level data
                    rowPitch,                          // Row pitch of source image (should match DDS format)
                    nullptr                            // No subrectangle (tile the full image)
                );

                ddsOffset += ddsMipLevelSize;
            }
        }

        void Image_Replace_Cube(GfxImage *image, const DDSImage &ddsImage)
        {
            if (image->mapType != MAPTYPE_CUBE)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a cube map!\n", image->name);
                return;
            }

            const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);

            // Check can we get the base size here and /6 for face size
            unsigned int face_size = 0;
            unsigned int rowPitch = 0;

            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
                face_size = (image->width / 4) * (image->height / 4) * 8;
                rowPitch = (image->width / 4) * 8; // 8 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_8_8_8_8:
                face_size = image->width * image->height * 4;
                rowPitch = image->width * 4; // 4 bytes per pixel
                break;
            default:
                Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has unsupported format %d!\n", image->name, format);
                return;
            }

            for (int i = 0; i < 6; i++)
            {
                const unsigned char *face_pixels = ddsImage.data.data() + (i * face_size);

                // Create a buffer for the tiled texture data
                std::vector<uint8_t> tiledData(face_size);

                // Convert the linear texture to tiled format using XGTileTextureLevel
                XGTileTextureLevel(
                    image->width,               // Width
                    image->height,              // Height
                    0,                          // Level (base level)
                    static_cast<DWORD>(format), // GpuFormat
                    0,                          // Flags (no special flags)
                    tiledData.data(),           // pDestination (tiled output)
                    nullptr,                    // pPoint (no offset)
                    face_pixels,                // pSource (linear input)
                    rowPitch,                   // RowPitch
                    nullptr                     // pRect (entire texture)
                );

                GPUEndianSwapTexture(tiledData, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                // Copy the data to the image
                memcpy(image->pixels + (i * face_size), tiledData.data(), face_size);
            }
        }

        void Image_Replace(GfxImage *image)
        {
            const std::string replacement_base_dir = "game:\\raw\\images";
            const std::string replacement_path = replacement_base_dir + "\\" + image->name + ".dds";

            if (!filesystem::file_exists(replacement_path))
            {
                Com_PrintError(CON_CHANNEL_ERROR, "File does not exist: %s\n", replacement_path.c_str());
                return;
            }

            DDSImage ddsImage = ReadDDSFile(replacement_path.c_str());
            if (ddsImage.data.empty())
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Failed to load DDS file: %s\n", replacement_path.c_str());
                return;
            }

            if (image->width != ddsImage.header.width || image->height != ddsImage.header.height)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' dimensions do not match DDS file: %s\n", image->name, replacement_path.c_str());
                return;
            }

            GPUTEXTUREFORMAT ddsFormat;
            switch (ddsImage.header.pixelFormat.fourCC)
            {
            case DXT1_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXT1;
                break;
            case DXT3_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXT2_3;
                break;
            case DXT5_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXT4_5;
                break;
            case DXN_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXN;
                break;
            default:
                Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has an unsupported DDS format: 0x%X\n", image->name, ddsImage.header.pixelFormat.fourCC);
                return;
            }

            if (static_cast<uint32_t>(image->texture.basemap->Format.DataFormat) != static_cast<uint32_t>(ddsFormat))
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' format does not match DDS file: Expected %d, Got %d\n",
                               image->name, static_cast<uint32_t>(image->texture.basemap->Format.DataFormat),
                               static_cast<uint32_t>(ddsFormat));
                return;
            }

            if (image->mapType == MAPTYPE_2D)
            {
                Image_Replace_2D(image, ddsImage);
            }
            else if (image->mapType == MAPTYPE_CUBE)
            {
                Image_Replace_Cube(image, ddsImage);
            }
            else
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a 2D or cube map!\n", image->name);
                return;
            }
        }

        void Load_images()
        {
            const UINT MAX_IMAGES = 2048;
            XAssetHeader assets[MAX_IMAGES];
            UINT count = DB_GetAllXAssetOfType_FastFile(ASSET_TYPE_IMAGE, assets, MAX_IMAGES);
            DbgPrint("Cmd_imageload2_f: Found %d images\n", count);
            for (UINT i = 0; i < count; i++)
            {
                GfxImage *image = assets[i].image;
                // debug image metadata print out all
                Image_Replace(image);
            }
        }

        Detour CG_RegisterGraphics_Detour;

        void CG_RegisterGraphics_Hook(int localClientNum, const char *mapname)
        {
            CG_RegisterGraphics_Detour.GetOriginal<decltype(CG_RegisterGraphics)>()(localClientNum, mapname);
            DbgPrint("CG_RegisterGraphics mapname=%s \n", mapname);
            Load_images();
        }

        Detour Cmd_ExecFromFastFile_Detour;

        bool Cmd_ExecFromFastFile_Hook(int localClientNum, int controllerIndex, const char *filename)
        {
            std::string file_path = "game:\\raw\\";
            file_path += filename;

            if (filesystem::file_exists(file_path))
            {
                std::string contents = filesystem::read_file_to_string(file_path);
                if (!contents.empty())
                {
                    Com_Printf(CON_CHANNEL_SYSTEM, "execing %s from raw:\\\n", filename);
                    Cbuf_ExecuteBuffer(localClientNum, controllerIndex, contents.c_str());
                    return true;
                }
            }

            return Cmd_ExecFromFastFile_Detour.GetOriginal<decltype(Cmd_ExecFromFastFile)>()(localClientNum, controllerIndex, filename);
        }

        const float colorWhiteRGBA[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        void DrawFixedFPS()
        {
            static dvar_s *pm_fixed_fps = Dvar_FindMalleableVar("pm_fixed_fps");

            char buff[16];
            sprintf_s(buff, "%d", pm_fixed_fps->current.integer);

            static Font_s *font = R_RegisterFont("fonts/bigDevFont");
            float x = 620 * scrPlaceFullUnsafe.scaleVirtualToFull[0];
            float y = 15 * scrPlaceFullUnsafe.scaleVirtualToFull[1];
            R_AddCmdDrawText(buff, 16, font, x, y, 1.0, 1.0, 0.0, colorWhiteRGBA, 0);
        }

        void CG_DrawTAS()
        {
            static Font_s *bigDevFont = R_RegisterFont("fonts/bigDevFont");
            const float x = 10.f * scrPlaceFullUnsafe.scaleVirtualToFull[0];
            const float y = 30.f;
            R_AddCmdDrawText("TAS", 5, bigDevFont, x, y, 1.0, 1.0, 0.0, colorWhiteRGBA, 0);
        }

        dvar_s *cg_draw_player_info = nullptr;

        void CG_DrawPlayerInfo()
        {
            auto ps = CG_GetPredictedPlayerState(0);
            int speed2D = static_cast<int>(sqrtf(ps->velocity[0] * ps->velocity[0] + ps->velocity[1] * ps->velocity[1]));

            char buff[256];
            sprintf_s(buff,
                      "x: %.6f\n"
                      "y: %.6f\n"
                      "z: %.6f\n"
                      "pitch: %.6f\n"
                      "yaw: %.6f\n"
                      "speed: %d\n",
                      ps->origin[0],
                      ps->origin[1],
                      ps->origin[2],
                      ps->viewangles[0],
                      ps->viewangles[1],
                      speed2D);

            static Font_s *consoleFont = R_RegisterFont("fonts/consoleFont");
            const float x = 10.f * scrPlaceFullUnsafe.scaleVirtualToFull[0];
            const float y = 50.f;

            R_AddCmdDrawText(buff, 256, consoleFont, x, y, 1.0, 1.0, 0.0, colorWhiteRGBA, 0);
        }

        Detour CG_DrawActive_Detour;

        void CG_DrawActive_Hook(int localClientNum)
        {
            static dvar_s *pm_fixed_fps_enable = Dvar_FindMalleableVar("pm_fixed_fps_enable");

            CheckKeyboardCompletion();

            if (pm_fixed_fps_enable->current.enabled)
            {
                DrawFixedFPS();
            }

            if (cj_tas::TAS_Enabled())
            {
                CG_DrawTAS();
            }

            if (cg_draw_player_info->current.enabled)
            {
                CG_DrawPlayerInfo();
            }

            clipmap::HandleBrushCollisionChange();

            CG_DrawActive_Detour.GetOriginal<decltype(CG_DrawActive)>()(localClientNum);
        }

        Detour UI_Refresh_Detour;

        void UI_Refresh_Hook(int localClientNum)
        {
            UI_Refresh_Detour.GetOriginal<decltype(UI_Refresh)>()(localClientNum);
            CheckKeyboardCompletion();
        }

        void GScr_CloneBrushModelToScriptModel(scr_entref_t scriptModelEntRef)
        {
            gentity_s *scriptEnt = GetEntity(scriptModelEntRef);
            gentity_s *brushEnt = Scr_GetEntity(0);

            SV_UnlinkEntity(scriptEnt);
            scriptEnt->s.index = brushEnt->s.index;
            int contents = scriptEnt->r.contents;
            SV_SetBrushModel(scriptEnt);
            scriptEnt->r.contents |= contents;
            SV_LinkEntity(scriptEnt);
        }

        struct BotAction
        {
            bool jump;
        };

        // map of client index to bot action
        std::map<int, BotAction> botActions;

        void GScr_BotJump(scr_entref_t entref)
        {
            client_t *cl = &svsHeader->clients[entref.entnum];

            if (cl->header.state && cl->header.netchan.remoteAddress.type == NA_BOT)
            {
                botActions[entref.entnum].jump = true;
            }
        }

        Detour SV_ClientThinkDetour;

        // TODO: maybe recreate the original and call it in the hook
        void SV_ClientThinkHook(client_t *cl, usercmd_s *cmd)
        {
            // Check if the client is a bot
            if (cl->header.state && cl->header.netchan.remoteAddress.type == NA_BOT)
            {
                // Reset bot's movement and actions set in SV_BotUserMove
                cmd->forwardmove = 0;
                cmd->rightmove = 0;
                cmd->buttons = 0;

                int clientIndex = cl - svsHeader->clients;
                if (botActions.find(clientIndex) != botActions.end())
                {
                    if (botActions[clientIndex].jump)
                    {
                        cmd->buttons = 1024; // BUTTON_JUMP
                        botActions[clientIndex].jump = false;
                    }
                }
            }

            SV_ClientThinkDetour.GetOriginal<decltype(SV_ClientThink)>()(cl, cmd);
        }

        void PlayerCmd_HoldBreathButtonPressed(scr_entref_t entref)
        {
            gentity_s *ent = GetEntity(entref);
            Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & 8192) != 0);
        }

        void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
        {
            gentity_s *ent = GetEntity(entref);
            Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & 1024) != 0);
        }

        void PlayerCmd_GetForwardMove(scr_entref_t entref)
        {
            client_t *cl = &svsHeader->clients[entref.entnum];
            Scr_AddInt(cl->lastUsercmd.forwardmove);
        }

        void PlayerCmd_GetRightMove(scr_entref_t entref)
        {
            client_t *cl = &svsHeader->clients[entref.entnum];
            Scr_AddInt(cl->lastUsercmd.rightmove);
        }

        void PlayerCmd_SetVelocity(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity");

            auto ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum));

            if (Scr_GetNumParam() != 1)
                Scr_Error("Usage: <client> SetVelocity( vec3 )\n");

            float velocity[3] = {0};

            Scr_GetVector(0, velocity);

            ent->client->ps.velocity[0] = velocity[0];
            ent->client->ps.velocity[1] = velocity[1];
            ent->client->ps.velocity[2] = velocity[2];
        }

        void PlayerCmd_NightVisionButtonPressed(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity");

            auto ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum));

            if (Scr_GetNumParam())
                Scr_Error("Usage: <client> NightVisionButtonPressed()\n");

            Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & 262144) != 0);
        }

        Detour Scr_GetMethod_Detour;

        BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
        {

            if (std::strcmp(*pName, "botjump") == 0)
                return reinterpret_cast<BuiltinMethod>(&GScr_BotJump);

            if (std::strcmp(*pName, "clonebrushmodeltoscriptmodel") == 0)
                return reinterpret_cast<BuiltinMethod>(&GScr_CloneBrushModelToScriptModel);

            if (std::strcmp(*pName, "holdbreathbuttonpressed") == 0)
                return reinterpret_cast<BuiltinMethod>(&PlayerCmd_HoldBreathButtonPressed);

            if (std::strcmp(*pName, "jumpbuttonpressed") == 0)
                return reinterpret_cast<BuiltinMethod>(&PlayerCmd_JumpButtonPressed);

            if (std::strcmp(*pName, "getforwardmove") == 0)
                return reinterpret_cast<BuiltinMethod>(&PlayerCmd_GetForwardMove);

            if (std::strcmp(*pName, "getrightmove") == 0)
                return reinterpret_cast<BuiltinMethod>(&PlayerCmd_GetRightMove);

            if (std::strcmp(*pName, "setvelocity") == 0)
                return reinterpret_cast<BuiltinMethod>(&PlayerCmd_SetVelocity);

            if (std::strcmp(*pName, "nightvisionbuttonpressed") == 0)
                return reinterpret_cast<BuiltinMethod>(&PlayerCmd_NightVisionButtonPressed);

            return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
        }

        void PatchViewpos()
        {
            // viewpos patches
            // Force the viewpos output to be printed to the obituary channel
            *(volatile uint32_t *)0x82320D2C = 0x38600005;

            // Update viewpos output format to display float values with 6 decimal places
            // Original format: "(%.0f %.0f %.0f) : %.0f %.0f\n"
            const char *newFormat = "%.6f, %.6f, %.6f ,%.6f, %.6f\n";
            memcpy((void *)0x8204F7DC, newFormat, strlen(newFormat) + 1); // Include null terminator
        }

        Detour Pmove_Detour;

        // https://github.com/kejjjjj/iw3sptool/blob/17b669233a1ad086deed867469dc9530b84c20e6/iw3sptool/bg/bg_pmove.cpp#L11
        void Pmove_Hook(pmove_t *pm)
        {
            static dvar_s *pm_fixed_fps_enable = Dvar_FindMalleableVar("pm_fixed_fps_enable");
            static dvar_s *pm_fixed_fps = Dvar_FindMalleableVar("pm_fixed_fps");

            if (!pm_fixed_fps_enable->current.enabled)
                return Pmove_Detour.GetOriginal<decltype(Pmove)>()(pm);

            int msec = 0;
            int cur_msec = 1000 / pm_fixed_fps->current.integer;

            pm->cmd.serverTime = ((pm->cmd.serverTime + (cur_msec < 2 ? 2 : cur_msec) - 1) / cur_msec) * cur_msec;

            int finalTime = pm->cmd.serverTime;

            if (finalTime < pm->ps->commandTime)
            {
                return; // should not happen
            }

            if (finalTime > pm->ps->commandTime + 1000)
                pm->ps->commandTime = finalTime - 1000;
            pm->numtouch = 0;

            while (pm->ps->commandTime != finalTime)
            {
                msec = finalTime - pm->ps->commandTime;

                if (msec > cur_msec)
                    msec = cur_msec;

                pm->cmd.serverTime = msec + pm->ps->commandTime;
                PmoveSingle(pm);
                memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));
            }
        }

        std::vector<Module *> components;

        void RegisterComponent(Module *module)
        {
            DbgPrint("T4 MP: Component registered: %s\n", module->get_name());
            components.push_back(module);
        }

        void init()
        {
            DbgPrint("Initializing MP\n");

            RegisterComponent(new cg());
            RegisterComponent(new cj_tas());
            RegisterComponent(new clipmap());
            RegisterComponent(new cmds());
            RegisterComponent(new g_client_fields());
            RegisterComponent(new pm());
            RegisterComponent(new scr_vm_functions());

            UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
            UI_Refresh_Detour.Install();

            CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
            CG_DrawActive_Detour.Install();

            CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
            CL_ConsolePrint_Detour.Install();

            CL_GamepadButtonEvent_Detour = Detour(CL_GamepadButtonEvent, CL_GamepadButtonEvent_Hook);
            CL_GamepadButtonEvent_Detour.Install();

            Load_MapEntsPtr_Detour = Detour(Load_MapEntsPtr, Load_MapEntsPtr_Hook);
            Load_MapEntsPtr_Detour.Install();

            R_StreamLoadFileSynchronously_Detour = Detour(R_StreamLoadFileSynchronously, R_StreamLoadFileSynchronously_Hook);
            R_StreamLoadFileSynchronously_Detour.Install();

            Scr_ReadFile_FastFile_Detour = Detour(Scr_ReadFile_FastFile, Scr_ReadFile_FastFile_Hook);
            Scr_ReadFile_FastFile_Detour.Install();

            cmd_function_s *imagedump_VAR = new cmd_function_s;
            Cmd_AddCommandInternal("imagedump", Cmd_imagedump, imagedump_VAR);

            CG_RegisterGraphics_Detour = Detour(CG_RegisterGraphics, CG_RegisterGraphics_Hook);
            CG_RegisterGraphics_Detour.Install();

            Cmd_ExecFromFastFile_Detour = Detour(Cmd_ExecFromFastFile, Cmd_ExecFromFastFile_Hook);
            Cmd_ExecFromFastFile_Detour.Install();

            cmd_function_s *cmdinput_VAR = new cmd_function_s;
            Cmd_AddCommandInternal("cmdinput", Cmd_cmdinput_f, cmdinput_VAR);

            Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
            Scr_GetMethod_Detour.Install();

            SV_ClientThinkDetour = Detour(SV_ClientThink, SV_ClientThinkHook);
            SV_ClientThinkDetour.Install();

            PatchViewpos();

            Dvar_RegisterBool("pm_fixed_fps_enable", false, 0, "Enable fixed FPS mode");
            Dvar_RegisterInt("pm_fixed_fps", 250, 0, 1000, 0, "Fixed FPS value");
            Pmove_Detour = Detour(Pmove, Pmove_Hook);
            Pmove_Detour.Install();

            cg_draw_player_info = Dvar_RegisterBool("cg_draw_player_info", false, 0, "Draw player info (origin, viewangles, speed) on screen");
        }

        void shutdown()
        {
            DbgPrint("Shutting down MP\n");

            CG_DrawActive_Detour.Remove();
            CL_ConsolePrint_Detour.Remove();
            CL_GamepadButtonEvent_Detour.Remove();
            Load_MapEntsPtr_Detour.Remove();
            R_StreamLoadFileSynchronously_Detour.Remove();
            Scr_ReadFile_FastFile_Detour.Remove();
            Scr_GetMethod_Detour.Remove();
            SV_ClientThinkDetour.Remove();
            Pmove_Detour.Remove();

            // TODO: move module loader/unloader logic to a self contained class
            // Clean up in reverse order
            for (auto it = components.rbegin(); it != components.rend(); ++it)
            {
                delete *it;
            }
            components.clear();
        }

    }
}
