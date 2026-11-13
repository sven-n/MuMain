#include "doctest.h"

#include "Core/Platform/PlatformCrypto.h"
#include "Core/Utilities/CpuUsage.h"
#include "GameShop/FileDownloader/ShopDownload.h"

#include <array>
#include <filesystem>

// The adapters a target without these services compiles, linked here instead of the desktop ones.

TEST_CASE("A target without a credential store saves no login [core][platform-services]")
{
    constexpr std::array<std::uint8_t, 4> plaintext = {1, 2, 3, 4};
    std::vector<std::uint8_t> output = {9};

    CHECK_FALSE(mu_credentials_available());
    CHECK_FALSE(mu_encrypt_blob(plaintext.data(), plaintext.size(), output));
    CHECK(output.empty());
    CHECK_FALSE(mu_decrypt_blob(plaintext.data(), plaintext.size(), output));
    CHECK_FALSE(mu_decrypt_legacy_blob(plaintext.data(), plaintext.size(), output));
}

TEST_CASE("A target without shop downloads fails them and writes no file [core][platform-services]")
{
    const std::filesystem::path target =
        std::filesystem::temp_directory_path() / "mu_shop_download_unavailable" / "banner.jpg";
    std::filesystem::remove_all(target.parent_path());

    const std::atomic_bool cancel = false;
    WZResult result =
        ShopDownload::DownloadFile(L"http://example.invalid/banner.jpg", target.wstring(), L"", L"", true, &cancel);

    CHECK_FALSE(result.IsSuccess());
    CHECK(result.GetErrorCode() == DL_UNAVAILABLE);
    CHECK_FALSE(std::filesystem::exists(target.parent_path()));
}

TEST_CASE("A target that can't measure CPU use reports no sample [core][platform-services]")
{
    CHECK_FALSE(CpuUsage::Instance()->GetUsage().has_value());
}
