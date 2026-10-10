#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "FloppyEmulator.h"

using Compucolor::Impl::Floppy::FloppyEmulator;

namespace
{
    std::string TempPath(const std::string& name)
    {
        const auto directory = std::filesystem::temp_directory_path() / "cc2emu_floppy_tests";
        std::filesystem::create_directories(directory);

        return (directory / name).string();
    }

    // 41 tracks is not required; the loader only needs the tracks present.
    std::string WriteBlankImage(const std::string& name)
    {
        const std::string path = TempPath(name);
        std::ofstream file(path);

        file << "Compucolor Virtual Floppy Disk Image\n";
        file << "Label Test Disk\n";
        file << "Track 0\n";

        for (int line = 0; line < 60; line++)
        {
            file << std::string(64, '0') << '\n';
        }

        return path;
    }
}

TEST(FloppyEmulator, StartsWithNoDiskMounted) {
    //Arrange
    const FloppyEmulator floppy(nullptr);

    //Assert
    EXPECT_FALSE(floppy.IsMounted());
    EXPECT_FALSE(floppy.IsModified());
    EXPECT_EQ("", floppy.GetMountedPath());
}

TEST(FloppyEmulator, MountLoadsImageAndRecordsPath) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    const std::string path = WriteBlankImage("mount.ccvf");
    std::string error;

    //Act
    const bool mounted = floppy.Mount(path, error);

    //Assert
    EXPECT_TRUE(mounted) << error;
    EXPECT_TRUE(floppy.IsMounted());
    EXPECT_FALSE(floppy.IsModified());
    EXPECT_EQ(path, floppy.GetMountedPath());
}

TEST(FloppyEmulator, MountMissingFileFailsAndKeepsCurrentDisk) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    const std::string path = WriteBlankImage("keep.ccvf");
    std::string error;
    ASSERT_TRUE(floppy.Mount(path, error));

    //Act
    const bool mounted = floppy.Mount(TempPath("missing.ccvf"), error);

    //Assert
    EXPECT_FALSE(mounted);
    EXPECT_FALSE(error.empty());
    EXPECT_TRUE(floppy.IsMounted());
    EXPECT_EQ(path, floppy.GetMountedPath());
}

TEST(FloppyEmulator, MountMalformedImageFailsAndKeepsCurrentDisk) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    const std::string good = WriteBlankImage("good.ccvf");
    const std::string bad = TempPath("bad.ccvf");
    std::ofstream(bad) << "Track 0\nnot-hex\n";
    std::string error;
    ASSERT_TRUE(floppy.Mount(good, error));

    //Act
    const bool mounted = floppy.Mount(bad, error);

    //Assert
    EXPECT_FALSE(mounted);
    EXPECT_EQ(good, floppy.GetMountedPath());
}

TEST(FloppyEmulator, EjectClearsMountState) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    std::string error;
    ASSERT_TRUE(floppy.Mount(WriteBlankImage("eject.ccvf"), error));

    //Act
    floppy.Eject();

    //Assert
    EXPECT_FALSE(floppy.IsMounted());
    EXPECT_EQ("", floppy.GetMountedPath());
}

TEST(FloppyEmulator, SaveWithoutDiskFails) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    std::string error;

    //Act
    const bool saved = floppy.Save(error);

    //Assert
    EXPECT_FALSE(saved);
    EXPECT_FALSE(error.empty());
}

TEST(FloppyEmulator, SaveAsWritesImageThatCanBeMountedAgain) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    std::string error;
    ASSERT_TRUE(floppy.Mount(WriteBlankImage("source.ccvf"), error));
    const std::string target = TempPath("copy.ccvf");

    //Act
    const bool saved = floppy.SaveAs(target, error);
    FloppyEmulator other(nullptr);
    const bool remounted = other.Mount(target, error);

    //Assert
    EXPECT_TRUE(saved) << error;
    EXPECT_EQ(target, floppy.GetMountedPath());
    EXPECT_TRUE(remounted) << error;
}
