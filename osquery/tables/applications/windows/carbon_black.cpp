/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>

#include <osquery/core.h>
#include <osquery/filesystem.h>
#include <osquery/tables.h>

#include "osquery/tables/system/windows/registry.h"

namespace fs = boost::filesystem;

namespace osquery {
namespace tables {

// Carbon Black registry path
#define kCbRegLoc "SOFTWARE\\CarbonBlack\\config"
// Path to Carbon Black direcotry
#define kCbDir "C:\\Windows\\CarbonBlack\\"

void getQueue(Row& r) {
  std::vector<std::string> files_list;
  auto status = listFilesInDirectory(kCbDir, files_list, true);
  if (!status.ok()) {
    return;
  }
  unsigned int binary_queue_size = 0;
  unsigned int event_queue_size = 0;
  // Go through each file
  for (const auto& kfile : files_list) {
    fs::path file(kfile);
    if (file.filename() == "data" || file.filename() == "info.txt") {
      binary_queue_size += fs::file_size(kfile);
    }
    if (file.filename() == "active-event.log" ||
        boost::starts_with(file.filename().c_str(), "eventlog_")) {
      event_queue_size += fs::file_size(kfile);
    }
  }
  r["binary_queue"] = INTEGER(binary_queue_size);
  r["event_queue"] = INTEGER(event_queue_size);
}

void getSettings(Row& r) {
  QueryData results;
  queryKey("HKEY_LOCAL_MACHINE", kCbRegLoc, results);
  for (const auto& kKey : results) {
    if (kKey.at("name") == "CollectCrossProcess") {
      r["collect_cross_processes"] = SQL_TEXT(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectStoreFiles") {
      r["collect_store_files"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectDataFileWrites") {
      r["collect_data_file_writes"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectEmetEvents") {
      r["collect_emet_events"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectFileMods") {
      r["collect_file_mods"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectModuleInfo") {
      r["collect_module_info"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectModuleLoads") {
      r["collect_module_loads"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectNetConns") {
      r["collect_net_conns"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectProcesses") {
      r["collect_processes"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectProcessUserContext") {
      r["collect_process_user_context"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectRegMods") {
      r["collect_reg_mods"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectSensorOperations") {
      r["collect_sensor_operations"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "CollectStoreFiles") {
      r["collect_store_files"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "ConfigName") {
      r["config_name"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "LogFileDiskQuotaMb") {
      r["log_file_disk_quota_mb"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "LogFileDiskQuotaPercentage") {
      r["log_file_disk_quota_percentage"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "ProtectionDisabled") {
      r["protection_disabled"] = INTEGER(kKey.at("data"));
    }
    if (kKey.at("name") == "SensorIpAddr") {
      r["sensor_ip_addr"] = SQL_TEXT(kKey.at("data"));
    }
    if (kKey.at("name") == "SensorBackendServer") {
      std::string server = kKey.at("data");
      boost::replace_all(server, "%3A", ":");
      r["sensor_backend_server"] = SQL_TEXT(server);
    }
    if (kKey.at("name") == "SensorId") {
      // from a string to an int, to hex, a portion of the hex, then to int
      uint64_t int_sensor_id = strtoll(kKey.at("data").c_str(), NULL, 10);
      std::stringstream hex_sensor_id;
      hex_sensor_id << std::hex << int_sensor_id;
      unsigned int sensor_id;
      std::string small_hex_sensor_id = hex_sensor_id.str().substr(11, 16);
      std::stringstream converter(small_hex_sensor_id);
      converter >> std::hex >> sensor_id;
      r["sensor_id"] = INTEGER(sensor_id);
    }
  }
}

QueryData genCarbonBlackInfo(QueryContext& context) {
  Row r;
  QueryData results;

  getSettings(r);
  getQueue(r);
  results.push_back(r);

  return results;
}
}
}
