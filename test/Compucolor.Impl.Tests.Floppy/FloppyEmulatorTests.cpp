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


TEST(FloppyEmulator, SaveWritesBackToTheMountedPathAndKeepsLabels) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    const std::string path = WriteBlankImage("save.ccvf");
    std::string error;
    ASSERT_TRUE(floppy.Mount(path, error));
    std::filesystem::remove(path);

    //Act
    const bool saved = floppy.Save(error);

    //Assert
    EXPECT_TRUE(saved) << error;
    EXPECT_FALSE(floppy.IsModified());

    std::ifstream file(path);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_NE(std::string::npos, contents.find("Label Test Disk"));
    EXPECT_NE(std::string::npos, contents.find("Track 0"));
}

TEST(FloppyEmulator, EjectedDriveReturnsNoData) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    std::string error;
    ASSERT_TRUE(floppy.Mount(WriteBlankImage("nodata.ccvf"), error));
    floppy.Eject();

    //Act
    floppy.Select(true, false, 0);
    floppy.Step(5'000'000);

    //Assert
    EXPECT_FALSE(floppy.HasData());
}


namespace
{
    void StepThrough(FloppyEmulator& floppy, std::initializer_list<uint8_t> phases)
    {
        for (const uint8_t phase : phases)
        {
            floppy.Select(true, false, phase);
        }
    }
}

TEST(FloppyEmulator, FourPhaseStepperNeedsTwoStepsPerTrack) {
    //Arrange
    FloppyEmulator floppy(nullptr);

    //Act: five steps in
    StepThrough(floppy, {1, 2, 4, 0, 1});

    //Assert
    EXPECT_EQ(2, floppy.GetTrack());
}

TEST(FloppyEmulator, ThreePhaseStepperMovesOneTrackPerStep) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    floppy.SetStepperPhases(3);

    //Act: 1 is the starting position, then three steps in
    StepThrough(floppy, {1, 2, 4, 1});

    //Assert
    EXPECT_EQ(3, floppy.GetTrack());
}

TEST(FloppyEmulator, ThreePhaseStepperStepsOutInTheOppositeOrder) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    floppy.SetStepperPhases(3);
    StepThrough(floppy, {1, 2, 4, 1});

    //Act
    StepThrough(floppy, {4, 2});

    //Assert
    EXPECT_EQ(1, floppy.GetTrack());
}

TEST(FloppyEmulator, ThreePhaseStepperIgnoresPhaseZero) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    floppy.SetStepperPhases(3);
    StepThrough(floppy, {1, 2});

    //Act: a 4 phase motor would step here
    StepThrough(floppy, {0, 4});

    //Assert: only 1->2 and 2->4 count
    EXPECT_EQ(2, floppy.GetTrack());
}

TEST(FloppyEmulator, ThreePhaseStepperStopsAtTrackForty) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    floppy.SetStepperPhases(3);

    //Act
    for (int step = 0; step < 60; step++)
    {
        StepThrough(floppy, {1, 2, 4});
    }

    //Assert
    EXPECT_EQ(40, floppy.GetTrack());
}

TEST(FloppyEmulator, ChangingStepperTypeParksTheHeadAtTrackZero) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    StepThrough(floppy, {1, 2, 4, 0, 1, 2, 4, 0});

    //Act
    floppy.SetStepperPhases(3);

    //Assert
    EXPECT_EQ(0, floppy.GetTrack());
}


TEST(FloppyEmulator, WritesFromTheMachineMarkTheDiskModifiedAndReachTheSavedImage) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    std::string error;
    ASSERT_TRUE(floppy.Mount(WriteBlankImage("machine_write.ccvf"), error));
    ASSERT_FALSE(floppy.IsModified());

    //Act: write enable, let a gap of ones pass under the head, then write a byte
    floppy.Select(true, true, 0);
    floppy.Step(1'000'000);
    floppy.Write(0, 0xA5);

    //Assert
    EXPECT_TRUE(floppy.IsModified());

    const std::string target = TempPath("machine_write_saved.ccvf");
    ASSERT_TRUE(floppy.SaveAs(target, error)) << error;
    EXPECT_FALSE(floppy.IsModified());

    std::ifstream file(target);
    std::string line;
    bool trackZeroChanged = false;
    while (std::getline(file, line))
    {
        if (line.size() == 64 && line != std::string(64, '0'))
        {
            trackZeroChanged = true;
            break;
        }
    }
    EXPECT_TRUE(trackZeroChanged);
}

TEST(FloppyEmulator, WritingToAnEmptyDriveDoesNotMarkItModified) {
    //Arrange
    FloppyEmulator floppy(nullptr);

    //Act
    floppy.Select(true, true, 0);
    floppy.Step(1'000'000);
    floppy.Write(0, 0xA5);

    //Assert
    EXPECT_FALSE(floppy.IsModified());
    EXPECT_FALSE(floppy.IsMounted());
}

TEST(FloppyEmulator, ReadingDoesNotMarkTheDiskModified) {
    //Arrange
    FloppyEmulator floppy(nullptr);
    std::string error;
    ASSERT_TRUE(floppy.Mount(WriteBlankImage("read_only.ccvf"), error));

    //Act
    floppy.Select(true, false, 0);
    floppy.Step(50'000'000);

    //Assert
    EXPECT_FALSE(floppy.IsModified());
}
