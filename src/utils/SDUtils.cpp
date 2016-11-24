#include "SDUtils.hpp"
#include <SD.h>

using namespace Tracker;

const String ROOT_PATH = "/";

void _deleteDir(File dir, String tempPath);

SDUtils::SDUtils() {
}

void SDUtils::setup() {
  if (!SD.begin(4, SPI_HALF_SPEED)) {
    Serial.println("✖ SD initialization error");
  } else {
    Serial.println("SD initialized OK");
  }
}

void SDUtils::deleteAllFilesOnCard() {
  Serial.println("Deleting all files on card");
  File root = SD.open(ROOT_PATH);
  if(root) {
    _deleteDir(root, ROOT_PATH);
  } else {
    Serial.println("Couldn't open root dir. No files will be deleted.");
  }
}

void _deleteDir(File dir, String tempPath) {
  Serial.printf("Deleting dir %s\n", dir.name());
  while(true) {
    File entry = dir.openNextFile();
    String localPath;

    // Serial.println("");
    if (entry) {
      //FIXME WHY THIS ISN'T FINDING THE FILES ON ROOT DIR?
      if (entry.isDirectory()) {
        localPath = tempPath + entry.name() + ROOT_PATH + '\0';
        char folderBuf[localPath.length()];
        localPath.toCharArray(folderBuf, localPath.length() );
        _deleteDir(entry, folderBuf);

        if(SD.rmdir(folderBuf)) {
          Serial.print("Deleted folder ");
          Serial.println(folderBuf);
          // FolderDeleteCount++;
        } else {
          Serial.print("Unable to delete folder ");
          Serial.println(folderBuf);
          // FailCount++;
        }
      } else {
        localPath = tempPath + entry.name() + '\0';
        char charBuf[localPath.length()];
        localPath.toCharArray(charBuf, localPath.length() );

        if(SD.remove(charBuf)) {
          Serial.print("Deleted file ");
          Serial.println(localPath);
          // DeletedCount++;
        } else {
          Serial.print("Failed to delete ");
          Serial.println(localPath);
          // FailCount++;
        }
      }
    } else {
      Serial.println("SD wipe finished");
      break;
    }
  }
}
