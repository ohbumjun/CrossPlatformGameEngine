#pragma once

#include "PathManager.h"

namespace HazelEditor
{
// Path Info 와 FileManager 는 Editor  코드로
// 넣어야 할 것 같다.
class FileManager
{
public:
    static void Initialize(const char* projectPath);

	static void Finalize();

    /**
	 * - 프로젝트 폴더 기준의 상대경로를 절대경로로 변환해주는 함수
	 * ex) relativePath 프로젝트 상대경로 ( "Resources/target.png" )
	 *							-> 절대경로 ( "C:/projects/test/Resources/target.png" )
	 * ex) usage FileManager::ToAbsolutePath("Resources/target.png")
	 */
    static std::string ToAbsolutePath(const char *relativePath);

	private:
    static std::string _projectPath;
};

} // namespace HazelEditor
