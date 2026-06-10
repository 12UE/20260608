

#include"../../Common/SocketInit.h"
#include"MySQLConnectionPool.h"
#include"../../Common/KCP.h"
#include"../../Common/Protocals.h"

int main() {
    SetConsoleOutputCP(936);
    KCP::GetInstance().Bind("192.168.3.3", 9527);
    std::cout << "服务器已启动..." << std::endl;
    MySQLConnectionPool pool("localhost", "root", "root", "school", 3306, 30);
    if (!pool.initialize()) {
        std::cerr << "初始化连接池失败" << std::endl;
        return 1;
    }else {
        std::cout << "连接池初始化成功" << std::endl;
    }
    while (true) {
        PacketHeader header;
        KCP::GetInstance().StickRecv((CHAR*)&header, sizeof(header));
        switch (header.m_PkgType) {
        case C2S_STUDENT: {
            if (header.m_op == QUERYPKG) {
                // 读取包体
                ClientStudentQueryPacket queryPacket;
                RecvRestPack(queryPacket, header);
                std::string SQLString = R"(
                        SELECT 
                            s.StudentID AS '学号',
                            s.StudentName AS '学生姓名',
                            c.ClassName AS '班级名字',
                            s.ClassID
                        FROM 
                            student s
                        LEFT JOIN 
                            class c ON s.ClassID = c.ClassID
                    )";

                // 添加WHERE子句（如果有条件）
                std::string whereClause;
                bool firstCondition = true;

                if (queryPacket.m_szStuName.m_nLength > 0) {
                    whereClause += "s.StudentName like '%" +
                        std::string(queryPacket.m_szStuName.m_szStr, queryPacket.m_szStuName.m_nLength) +
                        "%'";
                    firstCondition = false;
                }

                if (queryPacket.m_szCourseName.m_nLength > 0) {
                    if (!firstCondition) {
                        whereClause += " AND ";
                    }
                    whereClause += "c.ClassName like '%" +
                        std::string(queryPacket.m_szCourseName.m_szStr, queryPacket.m_szCourseName.m_nLength) +
                        "%'";
                }

                // 组合完整的SQL语句
                if (!whereClause.empty()) {
                    SQLString += " WHERE " + whereClause;
                }

                SQLString += " ORDER BY s.StudentID;";

                // 执行查询
                auto Futresult = pool.executeQueryAsync(SQLString);
                auto result = Futresult.get();

               
                int nBufferSize = sizeof(PkgS2CStudent) + result.rows.size() * sizeof(StudentInfo);
                char* pBuf = new char[nBufferSize] {0};
                PkgS2CStudent* pPkgS2CStudent = new(pBuf) PkgS2CStudent();

                // 同步完成后发送数据行
                for (int row = 0; row < result.rows.size(); row++) {
                    StudentInfo studentInfo(
                        atoi(result.rows[row][0].c_str()),
                        result.rows[row][1].c_str(),
                        result.rows[row][2].c_str(),
                        atoi(result.rows[row][3].c_str())
                    );
                    pPkgS2CStudent->Push(studentInfo);
                }
                // 发送学生信息包
                KCP::GetInstance().Send(pPkgS2CStudent, nBufferSize);

                break;
            }
            else if (header.m_op == DELETEPKG) {
                StudentDelete studelete;
                RecvRestPack(studelete, header);
                // 执行删除操作
                std::string SQLString = "DELETE FROM student WHERE StudentID = " + std::to_string(studelete.m_nStuID) + ";";
                auto result = pool.executeQuery(SQLString);
                if (result.success) {
                    std::cout << "Student with ID " << studelete.m_nStuID << " deleted successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to delete student: " << result.errorMessage << std::endl;
                }
                break;
            }
            else if (header.m_op == INSERTPKG) {
                StudentAdd stuadd;
                RecvRestPack(stuadd, header);
                // 执行插入操作
                std::string SQLString = "INSERT INTO student (StudentName, ClassID) VALUES ('" +
                    std::string(stuadd.m_szStuName.m_szStr, stuadd.m_szStuName.m_nLength) + "', " +
                    std::to_string(stuadd.m_CourseID) + ");";
                auto Futresult = pool.executeQueryAsync(SQLString);
                auto result = Futresult.get();
                if (result.success) {
                    std::cout << "Student " << stuadd.m_szStuName.m_szStr << " added successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to add student: " << result.errorMessage << std::endl;
                }
                break;
            }
            else if (header.m_op == UPDATEPKG) {
                StudentUpdate stuupdate;
                RecvRestPack(stuupdate, header);
                // 执行更新操作
                std::string SQLString = "UPDATE student SET StudentName = '" +
                    std::string(stuupdate.m_szStuName.m_szStr, stuupdate.m_szStuName.m_nLength) +
                    "', ClassID = " + std::to_string(stuupdate.m_CourseID) +
                    " WHERE StudentID = " + std::to_string(stuupdate.m_nStuID) + ";";
                auto futresult = pool.executeQueryAsync(SQLString);
                auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Student with ID " << stuupdate.m_nStuID << " updated successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to update student: " << result.errorMessage << std::endl;
                }
            }
            break;
        }
        case C2S_CLASS: {
            if (header.m_op == QUERYPKG) {
                ClassQueryPacket ClassQuery;
                RecvRestPack(ClassQuery, header);
                //查询全部
                std::string SQL = R"(SELECT 
                            c.ClassID,
                            c.ClassName,
                            COUNT(s.StudentID) AS StudentCount
                            FROM
                            class c
                            LEFT JOIN
                            student s ON c.ClassID = s.ClassID
                            WHERE c.ClassName LIKE '%)";
                SQL += std::string(ClassQuery.m_szClassName.m_szStr, ClassQuery.m_szClassName.m_nLength) + "%'";
                SQL += R"(
                            GROUP BY c.ClassID, c.ClassName
                            ORDER BY c.ClassID;)";
                auto futresult = pool.executeQueryAsync(SQL);
                auto result = futresult.get();  // 等待异步操作完成
                int nBufferSize = sizeof(PkgS2CClass) + result.rows.size() * sizeof(ClassInfo);
                char* pBuf = new char[nBufferSize] {0};
                PkgS2CClass* pPkgS2CStudent = new(pBuf) PkgS2CClass();
                // 同步完成后发送数据行
                for (int row = 0; row < result.rows.size(); row++) {
                    ClassInfo classinfo(
                        atoi(result.rows[row][0].c_str()),
                        result.rows[row][1].c_str(),
                        atoi(result.rows[row][2].c_str())
                    );
                    pPkgS2CStudent->Push(classinfo);
                }
                // 发送班级信息包
                KCP::GetInstance().Send(pPkgS2CStudent, nBufferSize);
                break;
            }
            else if (header.m_op == DELETEPKG) {
                ClassDelete classdelete;
                RecvRestPack(classdelete, header);
                // 执行删除操作
                std::string SQLString = "DELETE FROM class WHERE ClassID = " + std::to_string(classdelete.m_nClassID) + ";";
                auto result = pool.executeQuery(SQLString);
                if (result.success) {
                    std::cout << "Class with ID " << classdelete.m_nClassID << " deleted successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to delete class: " << result.errorMessage << std::endl;

                }
                break;
            }
            else if (header.m_op == INSERTPKG) {
				ClassAdd classadd;
				RecvRestPack(classadd, header);
				// 执行插入操作
                if(classadd.m_szClassName.m_nLength == 0) {
                    std::cerr << "[INSERT] Class name cannot be empty." << std::endl;
                    break;
				}
                std::string SQLString = "INSERT INTO class (ClassName) VALUES ('" +
					std::string(classadd.m_szClassName.m_szStr, classadd.m_szClassName.m_nLength) + "');";
				auto Futresult = pool.executeQueryAsync(SQLString);
				auto result = Futresult.get();
                if (result.success) {
                    std::cout << "Class " << classadd.m_szClassName.m_szStr << " added successfully." << std::endl;
                }
                else {
					std::cerr << "Failed to add class: " << result.errorMessage << std::endl;
                }

                break;
            }
            else if (header.m_op=UPDATEPKG)
            {
                ClassUpdate classupdate;
				RecvRestPack(classupdate, header);
                // 执行更新操作
                std::string SQLString = "UPDATE class SET ClassName = '" +
                    std::string(classupdate.m_ClassName.m_szStr, classupdate.m_ClassName.m_nLength) +
                    "' WHERE ClassID = " + std::to_string(classupdate.m_nClassID) + ";";
                auto futresult = pool.executeQueryAsync(SQLString);
                auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Class with ID " << classupdate.m_nClassID << " updated successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to update class: " << result.errorMessage << std::endl;
				}
                break;
            }
            break;
        }
        case C2S_COURSE:
            if (header.m_op == QUERYPKG) {
				ClientCourseQueryPacket courseQuery;
				RecvRestPack(courseQuery, header);
                // 基础 SQL
                std::string SQL = R"(
                SELECT 
                                    c.CourseID,
                                    c.CourseName,
                                    COUNT(sc.StudentID) AS StudentCount
                                FROM
                                    course c
                                LEFT JOIN
                                    studentcourse sc ON c.CourseID = sc.CourseID
                            )";
                std::string courseName(courseQuery.m_szCourseName.m_szStr, courseQuery.m_szCourseName.m_nLength);
                if (!courseName.empty()) {
                    SQL += " WHERE c.CourseName LIKE '" +courseName+ "%'";
                }
                SQL += R"(
                    GROUP BY c.CourseID, c.CourseName
                    ORDER BY c.CourseID;
                )";
                auto futresult = pool.executeQueryAsync(SQL);
                auto result = futresult.get();  // 等待异步操作完成
                int nBufferSize = sizeof(PkgS2CCourse) + result.rows.size() * sizeof(CourseInfo);
                char* pBuf = new char[nBufferSize] {0};
                PkgS2CCourse* pPkgS2CCourse = new(pBuf) PkgS2CCourse();
   
                for (int row = 0; row < result.rows.size(); row++) {
                    CourseInfo courseinfo(
                        atoi(result.rows[row][0].c_str()),
                        result.rows[row][1].c_str(),
                        atoi(result.rows[row][2].c_str())
                    );
                    pPkgS2CCourse->Push(courseinfo);
                }
                // 发送课程信息包
                KCP::GetInstance().Send(pPkgS2CCourse, nBufferSize);
            }
			else if (header.m_op == DELETEPKG) {
				CourseDelete coursedelete;
				RecvRestPack(coursedelete, header);
				// 执行删除操作
				std::string SQLString = "DELETE FROM course WHERE CourseID = " + std::to_string(coursedelete.m_nCourseID) + ";";
				auto futresult = pool.executeQueryAsync(SQLString);
				auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Course with ID " << coursedelete.m_nCourseID << " deleted successfully." << std::endl;
                }
                else {
					std::cerr << "Failed to delete course: " << result.errorMessage << std::endl;
                }
			}
            else if (header.m_op == INSERTPKG) {
				CourseAdd courseadd;
				RecvRestPack(courseadd, header);
				// 执行插入操作
                if (courseadd.m_szCourseName.m_nLength == 0) {
                    std::cerr << "[INSERT] Course name cannot be empty." << std::endl;
                    break;
				}
				std::string SQLString = "INSERT INTO course (CourseName) VALUES ('" +
					std::string(courseadd.m_szCourseName.m_szStr, courseadd.m_szCourseName.m_nLength) + "');";
				auto futresult = pool.executeQueryAsync(SQLString);
                auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Course " << courseadd.m_szCourseName.m_szStr << " added successfully." << std::endl;
                }
				else {
					std::cerr << "Failed to add course: " << result.errorMessage << std::endl;
                    }
            }
			else if (header.m_op == UPDATEPKG) {
				CourseUpdate courseupdate;
				RecvRestPack(courseupdate, header);
                std::string SQLString = "UPDATE course SET CourseName = '" +
                    std::string(courseupdate.m_szCourseName.m_szStr, courseupdate.m_szCourseName.m_nLength) +
					"' WHERE CourseID = " + std::to_string(courseupdate.m_nCourseID) + ";";
				auto futresult = pool.executeQueryAsync(SQLString);
                auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Course with ID " << courseupdate.m_nCourseID << " updated successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to update course: " << result.errorMessage << std::endl;
				}
            }
            break;
        case C2S_ENROLL:
            if (header.m_op == QUERYPKG) {
				EnrollQueryPacket enrollQuery;
				RecvRestPack(enrollQuery, header);
                // 基础 SQL
                std::string SQL = R"(
                    SELECT 
                    s.StudentName,
                    s.StudentID,
                    c.ClassName,
                    co.CourseName,
                    co.CourseID,
                    sc.Score
                FROM 
                    studentcourse sc
                JOIN 
                    student s ON sc.StudentID = s.StudentID
                JOIN 
                    class c ON s.ClassID = c.ClassID
                JOIN 
                    course co ON sc.CourseID = co.CourseID
                )";
                std::string stuName(enrollQuery.m_szStuName.m_szStr, enrollQuery.m_szStuName.m_nLength);
                std::string courseName(enrollQuery.m_szCourseName.m_szStr, enrollQuery.m_szCourseName.m_nLength);
                if (!stuName.empty() || !courseName.empty()) {
                    SQL += " WHERE ";
                    if (!stuName.empty()) {
                        SQL += "s.StudentName LIKE '" + stuName + "%'";
                    }
                    if (!courseName.empty()) {
                        if (!stuName.empty()) {
                            SQL += " AND ";
                        }
                        SQL += "co.CourseName LIKE '" + courseName + "%'";
                    }
                }
				SQL += " ORDER BY s.StudentID;";
                auto futresult = pool.executeQueryAsync(SQL);
                auto result = futresult.get();  // 等待异步操作完成
                int nBufferSize = sizeof(PkgS2CEnroll) + result.rows.size() * sizeof(EnrollInfo);
                char* pBuf = new char[nBufferSize] {0};
                PkgS2CEnroll* pPkgS2CEnroll = new(pBuf) PkgS2CEnroll();
   
                for (int row = 0; row < result.rows.size(); row++) {
                    EnrollInfo enrollinfo(
                        const_cast<char*>(result.rows[row][0].c_str()), //StudentID
                        result.rows[row][1].c_str(), // StudentName
                        result.rows[row][2].c_str(), // ClassName
                        result.rows[row][3].c_str(), // CourseName
                        result.rows[row][4].c_str(), // CourseID
                        result.rows[row][5].c_str() // Score
                    );
                    pPkgS2CEnroll->Push(enrollinfo);
                }
                // 发送选课信息包
				KCP::GetInstance().Send(pPkgS2CEnroll, nBufferSize);
            }
            else if (header.m_op == UPDATEPKG) {
                EnrollUpdate enrollUpdatePacket;
                RecvRestPack(enrollUpdatePacket, header);
                std::string SqlBasic=R"(
                    UPDATE studentcourse 
                    SET Score = ')";
                SqlBasic += std::string(enrollUpdatePacket.m_szScore.m_szStr, enrollUpdatePacket.m_szScore.m_nLength) +
                    "' WHERE StudentID = " + std::string(enrollUpdatePacket.m_szStudentId.m_szStr) +
                    " AND CourseID = " + std::string(enrollUpdatePacket.m_szCourseId.m_szStr) + ";";
                auto futresult = pool.executeQueryAsync(SqlBasic);
                auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Score for Student ID " << enrollUpdatePacket.m_szStudentId.m_szStr
                              << " in Course ID " << enrollUpdatePacket.m_szCourseId.m_szStr
                              << " updated successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to update score: " << result.errorMessage << std::endl;
                }
            }
            else if (header.m_op == DELETEPKG) {
                EnrollDelete enrollDeletepacket;
                RecvRestPack(enrollDeletepacket, header);
                std::string SQLString = "DELETE FROM studentcourse WHERE StudentID = '" +
                    std::string(enrollDeletepacket.m_szStudentId.m_szStr) + "' AND CourseID = '" +
					std::string(enrollDeletepacket.m_szCourseId.m_szStr) + "';";
                auto futresult = pool.executeQueryAsync(SQLString);
                auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Enrollment for Student ID " << enrollDeletepacket.m_szStudentId.m_szStr
                              << " in Course ID " << enrollDeletepacket.m_szCourseId.m_szStr
                              << " deleted successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to delete enrollment: " << result.errorMessage << std::endl;
                }
            }
            else if (header.m_op == INSERTPKG) {
				EnrollInsert enrollInsertPacket;
				RecvRestPack(enrollInsertPacket, header);
                std::string SqlBasic = R"(
                    INSERT INTO studentcourse (StudentID, CourseID, Score) 
                    VALUES (')";
                SqlBasic += std::string(enrollInsertPacket.m_szStudentId.m_szStr) + "', '" +
                    std::string(enrollInsertPacket.m_szCourseId.m_szStr) + "', '" +
                    std::string(enrollInsertPacket.m_szScore.m_szStr) + "');";
                auto futresult = pool.executeQueryAsync(SqlBasic);
                auto result = futresult.get();  // 等待异步操作完成
                if (result.success) {
                    std::cout << "Enrollment for Student ID " << enrollInsertPacket.m_szStudentId.m_szStr
                              << " in Course ID " << enrollInsertPacket.m_szCourseId.m_szStr
                              << " added successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to add enrollment: " << result.errorMessage << std::endl;
				}
            }
            break;
        case C2S_COMBINE:
            if (header.m_op == QUERYPKG) {
                CombineQueryPacket CombineQuery;
                RecvRestPack(CombineQuery, header);

                // 基础 SQL 查询
                std::string SQL = R"(
                        SELECT 
                            c.ClassName AS '班级',
                            c.ClassID AS '班级ID',
                            co.CourseName AS '课程',
                            co.CourseID AS '课程ID',
                            s.StudentName AS '学生',
                            s.StudentID AS '学生ID',
                            sc.Score AS '考试分数'
                        FROM 
                            studentcourse sc
                        LEFT JOIN
                            student s ON sc.StudentID = s.StudentID
                        LEFT JOIN
                            class c ON s.ClassID = c.ClassID
                        LEFT JOIN
                            course co ON sc.CourseID = co.CourseID
                    )";

                // 动态构建 WHERE 条件
                std::vector<std::string> conditions;

                // 必选条件：确保只返回有学生和成绩的记录
                conditions.push_back("s.StudentID IS NOT NULL");
                conditions.push_back("sc.Score IS NOT NULL");

                // 可选条件：按需添加
                std::string className(CombineQuery.m_szClassName.m_szStr, CombineQuery.m_szClassName.m_nLength);
                if (!className.empty()) {
                    conditions.push_back("c.ClassName LIKE '" + className + "%'");
                }

                std::string courseName(CombineQuery.m_szCourseName.m_szStr, CombineQuery.m_szCourseName.m_nLength);
                if (!courseName.empty()) {
                    conditions.push_back("co.CourseName LIKE '" + courseName + "%'");
                }

                std::string studentName(CombineQuery.m_szStuName.m_szStr, CombineQuery.m_szStuName.m_nLength);
                if (!studentName.empty()) {
                    conditions.push_back("s.StudentName LIKE '%" + studentName + "%'"); // 包含匹配
                }

                // 分数范围条件
                if (CombineQuery.m_szLowestScore.m_nLength > 0) {
                    conditions.push_back("sc.Score >= " + std::string(CombineQuery.m_szLowestScore.m_szStr, CombineQuery.m_szLowestScore.m_nLength));
                }
                if (CombineQuery.m_szHighestScore.m_nLength > 0) {
                    conditions.push_back("sc.Score <= " + std::string(CombineQuery.m_szHighestScore.m_szStr, CombineQuery.m_szHighestScore.m_nLength));
                }

                // 手动拼接 WHERE 子句
                if (!conditions.empty()) {
                    SQL += " WHERE ";
                    for (size_t i = 0; i < conditions.size(); ++i) {
                        if (i > 0) {
                            SQL += " AND ";
                        }
                        SQL += conditions[i];
                    }
                }

                SQL += " ORDER BY s.StudentID, c.ClassName, co.CourseName;";

                // 执行查询
                auto futresult = pool.executeQueryAsync(SQL);
                auto result = futresult.get();

                // 构造返回数据包
                int nBufferSize = sizeof(PkgS2CCombine) + result.rows.size() * sizeof(CombineInfo);
                char* pBuf = new char[nBufferSize] {0};
                PkgS2CCombine* pPkgS2CCombine = new(pBuf) PkgS2CCombine();

                for (int row = 0; row < result.rows.size(); row++) {
                    CombineInfo combineinfo(
                        result.rows[row][0].c_str(), // ClassName
                        result.rows[row][1].c_str(), // ClassID
                        result.rows[row][2].c_str(), // CourseName
                        result.rows[row][3].c_str(), // CourseID
                        result.rows[row][4].c_str(), // StudentName
                        result.rows[row][5].c_str(), // StudentId
                        result.rows[row][6].c_str()  // Score
                    );
                    pPkgS2CCombine->Push(combineinfo);
                }

                // 发送结果
                KCP::GetInstance().Send(pPkgS2CCombine, nBufferSize);

            }
            else if (header.m_op == CLEAR) {
                // Execute each truncate statement separately
                std::vector<std::string> truncateQueries = {
                    "SET FOREIGN_KEY_CHECKS = 0",
                    "TRUNCATE TABLE `class`",
                    "TRUNCATE TABLE `course`",
                    "TRUNCATE TABLE `student`",
                    "TRUNCATE TABLE `studentcourse`",
                    "SET FOREIGN_KEY_CHECKS = 1"
                };

                bool allSuccess = true;
                std::string errorMessage;

                for (const auto& query : truncateQueries) {
                    auto futresult = pool.executeQueryAsync(query);
                    auto result = futresult.get();
                    if (!result.success) {
                        allSuccess = false;
                        errorMessage = result.errorMessage;
                        break;
                    }
                }

                if (allSuccess) {
                    std::cout << "All data cleared successfully." << std::endl;
                }
                else {
                    std::cerr << "Failed to clear data: " << errorMessage << std::endl;
                }
            }
            break;
        default:
            std::cerr << "Unknown packet type received." << std::endl;
            break;
        }
    }
    return 0;
}