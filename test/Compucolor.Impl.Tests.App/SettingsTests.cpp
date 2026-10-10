#include <filesystem>

#include <gtest/gtest.h>

#include "Settings.h"

using Compucolor::Impl::App::Sfml::Settings;

namespace
{
    std::string TempPath(const std::string& name)
    {
        return (std::filesystem::temp_directory_path() / "cc2emu_tests" / name).string();
    }
}

TEST(Settings, DefaultsAreProvided) {
    //Arrange
    const Settings settings;

    //Assert
    EXPECT_EQ(2, settings.GetInt("window_scale"));
    EXPECT_FALSE(settings.GetBool("fullscreen"));
    EXPECT_TRUE(settings.GetBool("speed_regulated"));
    EXPECT_EQ("standard", settings.GetString("charset"));
}

TEST(Settings, UnknownKeyReturnsEmptyValues) {
    //Arrange
    const Settings settings;

    //Assert
    EXPECT_EQ("", settings.GetString("missing"));
    EXPECT_EQ(0, settings.GetInt("missing"));
    EXPECT_FALSE(settings.GetBool("missing"));
}

TEST(Settings, SaveThenLoadRoundTrips) {
    //Arrange
    const std::string path = TempPath("roundtrip.config");
    Settings saved;
    saved.SetInt("window_scale", 3);
    saved.SetBool("fullscreen", true);
    saved.SetString("disk_a_path", "/disks/life.ccvf");

    //Act
    ASSERT_TRUE(saved.Save(path));
    Settings loaded;
    ASSERT_TRUE(loaded.Load(path));

    //Assert
    EXPECT_EQ(3, loaded.GetInt("window_scale"));
    EXPECT_TRUE(loaded.GetBool("fullscreen"));
    EXPECT_EQ("/disks/life.ccvf", loaded.GetString("disk_a_path"));
}

TEST(Settings, LoadMissingFileKeepsDefaults) {
    //Arrange
    Settings settings;

    //Act
    const bool loaded = settings.Load(TempPath("does_not_exist.config"));

    //Assert
    EXPECT_FALSE(loaded);
    EXPECT_EQ(2, settings.GetInt("window_scale"));
}

TEST(Settings, ValueMayContainEqualsSign) {
    //Arrange
    const std::string path = TempPath("equals.config");
    Settings saved;
    saved.SetString("disk_b_path", "/a=b/disk.ccvf");
    ASSERT_TRUE(saved.Save(path));

    //Act
    Settings loaded;
    ASSERT_TRUE(loaded.Load(path));

    //Assert
    EXPECT_EQ("/a=b/disk.ccvf", loaded.GetString("disk_b_path"));
}
