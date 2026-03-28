#include "workload_parser_test_fixture.hpp"

TEST_F(WorkloadParserTest, ConstructorThrowsOnInvalidRoot) {
  EXPECT_THROW(WorkloadParser("/path/that/definitely/does/not/exist"),
               std::invalid_argument);
}

TEST_F(WorkloadParserTest, ParseThrowsOnMissingCsv) {
  WorkloadParser parser(tempRoot.string());

  // We haven't created the CSV yet, so this should throw
  EXPECT_THROW((void)parser.parse(WorkloadType::INTERACTIVE),
               std::runtime_error);
}

TEST_F(WorkloadParserTest, ParseSuccessfullyReadsValidCsv) {
  std::string csvContent =
      "ProcessID,Priority(1-10),BurstTime,ArrivalTime\n"
      "1,7,48,2\n"
      "2,9,43,6\n";

  createMockCsv(WorkloadType::BACKGROUND, csvContent);
  createMockTraceFile(WorkloadType::BACKGROUND, 1);
  createMockTraceFile(WorkloadType::BACKGROUND, 2);

  WorkloadParser parser(tempRoot.string());
  testing::internal::CaptureStderr();

  auto workloads = parser.parse(WorkloadType::BACKGROUND);
  std::string stderrOutput = testing::internal::GetCapturedStderr();

  EXPECT_TRUE(stderrOutput.empty())
      << "Expected no warnings, but got: " << stderrOutput;
  ASSERT_EQ(workloads.size(), 2u);

  EXPECT_EQ(workloads[0].mProcess.getId(), 1);
  EXPECT_EQ(workloads[0].mProcess.getPriority(), 7);
  EXPECT_EQ(workloads[0].mTraceFilePath.filename(), "process1.ref");

  EXPECT_EQ(workloads[1].mProcess.getId(), 2);
  EXPECT_EQ(workloads[1].mProcess.getPriority(), 9);
  EXPECT_EQ(workloads[1].mTraceFilePath.filename(), "process2.ref");
}

TEST_F(WorkloadParserTest, ParseTraceFileThrowsOnBadFormat) {
  WorkloadParser parser(tempRoot.string());

  fs::path badTracePath = tempRoot / "bad_trace.ref";
  std::ofstream out(badTracePath);
  out << "Z 1A2B\n";
  out.close();

  EXPECT_THROW((void)parser.parseTraceFile(badTracePath), std::runtime_error);
}

TEST_F(WorkloadParserTest, ParseHandlesMalformedLinesGracefully) {
  std::string csvContent =
      "ProcessID,Priority(1-10),BurstTime,ArrivalTime\n"
      "1,7,48,2\n"
      "BAD_DATA,xyz\n"
      "3,10,163,8\n";

  createMockCsv(WorkloadType::MIXED_INTERACTIVE_BACKGROUND, csvContent);
  createMockTraceFile(WorkloadType::MIXED_INTERACTIVE_BACKGROUND, 1);
  createMockTraceFile(WorkloadType::MIXED_INTERACTIVE_BACKGROUND, 3);

  WorkloadParser parser(tempRoot.string());
  testing::internal::CaptureStderr();

  auto workloads = parser.parse(WorkloadType::MIXED_INTERACTIVE_BACKGROUND);
  std::string stderrOutput = testing::internal::GetCapturedStderr();

  ASSERT_EQ(workloads.size(), 2u);
  EXPECT_EQ(workloads[0].mProcess.getId(), 1);
  EXPECT_EQ(workloads[1].mProcess.getId(), 3);

  EXPECT_TRUE(stderrOutput.find("Error parsing CSV line") != std::string::npos);
}

TEST_F(WorkloadParserTest, ParseLogsWarningForMissingTraceFile) {
  std::string csvContent =
      "ProcessID,Priority(1-10),BurstTime,ArrivalTime\n"
      "1,7,48,2\n";

  createMockCsv(WorkloadType::INTERACTIVE, csvContent);

  WorkloadParser parser(tempRoot.string());
  testing::internal::CaptureStderr();

  auto workloads = parser.parse(WorkloadType::INTERACTIVE);
  std::string stderrOutput = testing::internal::GetCapturedStderr();

  ASSERT_EQ(workloads.size(), 1u);

  EXPECT_TRUE(
      stderrOutput.find("Warning: Missing trace file for Process ID 1") !=
      std::string::npos);
}
