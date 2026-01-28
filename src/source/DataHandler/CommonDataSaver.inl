#pragma once

#ifdef _EDITOR

#include <sstream>
#include <vector>

// Helper: Load original file data for change comparison
// Supports auto-detection of legacy format if convertFromFileLegacy is provided
template<typename TRuntime, typename TFile>
std::unique_ptr<TRuntime[]> LoadOriginalDataForComparison(
    const typename CommonDataSaver::SaveConfig<TRuntime, TFile>& config,
    size_t fileStructSize,
    size_t itemCount)
{
    auto originalData = std::make_unique<TRuntime[]>(itemCount);
    memset(originalData.get(), 0, sizeof(TRuntime) * itemCount);

    FILE* fpOrig = _wfopen(config.fileName, L"rb");
    if (!fpOrig)
    {
        #ifdef _DEBUG
        OutputDebugStringW(L"[LoadOriginalData] File not found: ");
        OutputDebugStringW(config.fileName);
        OutputDebugStringW(L"\n");
        #endif
        return originalData;
    }

    #ifdef _DEBUG
    OutputDebugStringW(L"[LoadOriginalData] Successfully opened: ");
    OutputDebugStringW(config.fileName);
    OutputDebugStringW(L"\n");
    #endif

    // Get file size
    fseek(fpOrig, 0, SEEK_END);
    long fileSize = ftell(fpOrig);
    fseek(fpOrig, 0, SEEK_SET);

    // Calculate expected size for new format
    long expectedSizeNoChecksum = (long)(fileStructSize * itemCount);
    long expectedSizeWithChecksum = expectedSizeNoChecksum + sizeof(DWORD);

    // Check if legacy format handler is available and file size matches legacy
    bool isLegacyFormat = false;
    size_t actualStructSize = fileStructSize;

    if (config.legacyFileStructSize > 0 && config.convertFromFileLegacy)
    {
        long expectedLegacySizeNoChecksum = (long)(config.legacyFileStructSize * itemCount);
        long expectedLegacySizeWithChecksum = expectedLegacySizeNoChecksum + sizeof(DWORD);

        if (fileSize == expectedLegacySizeNoChecksum || fileSize == expectedLegacySizeWithChecksum)
        {
            isLegacyFormat = true;
            actualStructSize = config.legacyFileStructSize;
            #ifdef _DEBUG
            OutputDebugStringW(L"[LoadOriginalData] Legacy format detected\n");
            #endif
        }
    }

    #ifdef _DEBUG
    wchar_t debugMsg[256];
    swprintf_s(debugMsg, L"[LoadOriginalData] FileSize=%ld, NewFormat=%ld/%ld, Legacy=%s, ActualStructSize=%zu\n",
               fileSize, expectedSizeNoChecksum, expectedSizeWithChecksum,
               isLegacyFormat ? L"YES" : L"NO", actualStructSize);
    OutputDebugStringW(debugMsg);
    #endif

    // Check if file size matches either format
    long expectedNoChecksum = (long)(actualStructSize * itemCount);
    long expectedWithChecksum = expectedNoChecksum + sizeof(DWORD);

    if (fileSize != expectedNoChecksum && fileSize != expectedWithChecksum)
    {
        #ifdef _DEBUG
        OutputDebugStringW(L"[LoadOriginalData] File size mismatch - unknown format, skipping comparison\n");
        #endif
        fclose(fpOrig);
        return originalData;
    }

    // Read buffer using detected struct size
    auto origBuffer = std::make_unique<BYTE[]>(actualStructSize * itemCount);
    fread(origBuffer.get(), actualStructSize * itemCount, 1, fpOrig);
    fclose(fpOrig);

    // Decrypt entire buffer if needed
    if (config.decryptBuffer)
    {
        config.decryptBuffer(origBuffer.get(), actualStructSize * itemCount);
    }

    // Decrypt and convert per-record
    BYTE* pSeek = origBuffer.get();
    for (size_t i = 0; i < itemCount; i++)
    {
        if (config.decryptRecord)
        {
            config.decryptRecord(pSeek, actualStructSize);
        }

        // Convert file format to runtime format
        if (isLegacyFormat)
        {
            // Use legacy conversion
            config.convertFromFileLegacy(originalData[i], pSeek, actualStructSize);
        }
        else
        {
            // Use standard conversion
            TFile fileStruct;
            memset(&fileStruct, 0, fileStructSize);
            memcpy(&fileStruct, pSeek, actualStructSize);
            config.convertFromFile(originalData[i], fileStruct);
        }

        pSeek += actualStructSize;
    }

    return originalData;
}

// Helper: Convert runtime data to file format and track changes
template<typename TRuntime, typename TFile>
std::unique_ptr<BYTE[]> ConvertToFileFormatAndTrackChanges(
    const typename CommonDataSaver::SaveConfig<TRuntime, TFile>& config,
    const TRuntime* originalData,
    size_t fileStructSize,
    size_t itemCount,
    int& outChangeCount,
    std::stringstream& outChangeLog)
{
    auto buffer = std::make_unique<BYTE[]>(fileStructSize * itemCount);
    BYTE* pSeek = buffer.get();
    outChangeCount = 0;

    for (size_t i = 0; i < itemCount; i++)
    {
        TFile fileStruct;
        memset(&fileStruct, 0, fileStructSize);

        // Convert runtime to file format
        config.convertToFile(fileStruct, config.runtimeData[i]);

        // Track changes if comparison is enabled
        if (originalData && config.compareItems)
        {
            bool changed = false;
            std::stringstream itemChanges;

            config.compareItems(originalData[i], config.runtimeData[i], itemChanges, changed);

            if (changed)
            {
                std::string itemName = config.getItemName ?
                    config.getItemName((int)i, config.runtimeData[i]) :
                    std::to_string(i);
                outChangeLog << "[" << i << "] " << itemName << "\n" << itemChanges.str();
                outChangeCount++;
            }
        }

        // Encrypt per-record if needed (like BuxConvert)
        if (config.encryptRecord)
        {
            config.encryptRecord((BYTE*)&fileStruct, fileStructSize);
        }

        memcpy(pSeek, &fileStruct, fileStructSize);
        pSeek += fileStructSize;
    }

    return buffer;
}

// Helper: Write buffer to file with optional checksum
template<typename TRuntime, typename TFile>
bool WriteBufferToFile(
    const typename CommonDataSaver::SaveConfig<TRuntime, TFile>& config,
    BYTE* buffer,
    size_t bufferSize)
{
    // Encrypt entire buffer if needed
    if (config.encryptBuffer)
    {
        config.encryptBuffer(buffer, bufferSize);
    }

    // Generate checksum if needed
    DWORD checksum = 0;
    if (config.generateChecksum)
    {
        checksum = config.generateChecksum(buffer, bufferSize);
    }

    // Create backup
    CommonDataSaver::CreateBackup(config.fileName);

    // Open file for writing
    FILE* fp = _wfopen(config.fileName, L"wb");
    if (fp == NULL)
    {
        return false;
    }

    // Write buffer
    fwrite(buffer, bufferSize, 1, fp);

    // Write checksum if needed
    if (config.generateChecksum)
    {
        fwrite(&checksum, sizeof(DWORD), 1, fp);
    }

    fclose(fp);
    return true;
}

// Generic helper: Create standard save configuration with BuxConvert encryption
template<typename TRuntime, typename TFile>
typename CommonDataSaver::SaveConfig<TRuntime, TFile> CreateStandardSaveConfig(
    const wchar_t* fileName,
    size_t itemCount,
    const TRuntime* runtimeData,
    std::function<void(TFile&, const TRuntime&)> convertToFile,
    std::function<void(TRuntime&, const TFile&)> convertFromFile,
    std::function<void(const TRuntime&, const TRuntime&, std::stringstream&, bool&)> compareItems,
    std::function<std::string(int, const TRuntime&)> getItemName,
    DWORD checksumKey,
    std::string* outChangeLog)
{
    typename CommonDataSaver::SaveConfig<TRuntime, TFile> config;

    config.fileName = fileName;
    config.itemCount = itemCount;
    config.runtimeData = runtimeData;

    // Conversion functions
    config.convertToFile = convertToFile;
    config.convertFromFile = convertFromFile;

    // Encryption: BuxConvert per-record (standard for all MU data files)
    config.encryptRecord = [](BYTE* buffer, std::size_t size) {
        BuxConvert(buffer, size);
    };

    config.decryptRecord = [](BYTE* buffer, std::size_t size) {
        BuxConvert(buffer, size);
    };

    // Checksum with data-type-specific key
    config.generateChecksum = [checksumKey](BYTE* buffer, std::size_t size) -> DWORD {
        return GenerateCheckSum2(buffer, static_cast<DWORD>(size), checksumKey);
    };

    // Change tracking
    if (outChangeLog)
    {
        config.compareItems = compareItems;
        config.getItemName = getItemName;
        config.outChangeLog = outChangeLog;
    }

    return config;
}

// Main save orchestrator
template<typename TRuntime, typename TFile>
bool CommonDataSaver::SaveData(const SaveConfig<TRuntime, TFile>& config)
{
    const size_t fileStructSize = sizeof(TFile);
    const size_t itemCount = config.itemCount;

    // Load original file for comparison if change tracking is requested
    std::unique_ptr<TRuntime[]> originalData;
    if (config.outChangeLog && config.compareItems)
    {
        originalData = LoadOriginalDataForComparison<TRuntime, TFile>(
            config, fileStructSize, itemCount);
    }

    // Convert runtime data to file format and track changes
    std::stringstream changeLog;
    int changeCount = 0;
    auto buffer = ConvertToFileFormatAndTrackChanges<TRuntime, TFile>(
        config,
        originalData.get(),
        fileStructSize,
        itemCount,
        changeCount,
        changeLog);

    // Skip saving if no changes detected
    if (originalData && changeCount == 0)
    {
        if (config.outChangeLog)
        {
            *config.outChangeLog = "No changes detected.\n";
        }
        return false;
    }

    // Write to file
    bool success = WriteBufferToFile<TRuntime, TFile>(
        config,
        buffer.get(),
        fileStructSize * itemCount);

    if (!success)
    {
        return false;
    }

    // Generate change log
    if (originalData && config.outChangeLog && changeCount > 0)
    {
        *config.outChangeLog = "=== Changes (" + std::to_string(changeCount) +
            " items modified) ===\n" + changeLog.str();
    }

    return true;
}

#endif // _EDITOR
