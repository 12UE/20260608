#pragma pack(push)
#pragma pack(1)	
enum Operation {
	INSERTPKG,//插入包
	UPDATEPKG,//更新包
	DELETEPKG,//删除包
	QUERYPKG,//查询包
	RESPONSEPKG,//响应包
	CLEAR,
};
enum  PacketType{
	//学生信息
	C2S_STUDENT,
	S2C_STUDENT,
	//课程信息
	C2S_COURSE,
	S2C_COURSE,
	//选课信息
	C2S_ENROLL,
	S2C_ENROLL,
	//组合查询
	S2C_COMBINE,
	C2S_COMBINE,
	C2S_CLAER,
	//班级信息
	C2S_CLASS,
	S2C_CLASS,

};
struct PacketHeader {
	PacketHeader() = default;
	PacketHeader(PacketType type, Operation op, uint32_t length)
		: m_PkgType(type), m_op(op), m_length(length) {
	}
	PacketType m_PkgType;//包类型
	Operation  m_op;
	uint32_t   m_length; // 包长度
};

// 返回学生包信息
struct StudentInfo
{
	StudentInfo() = default;
	StudentInfo(int nStudentID, const char* szStudentName, const char* szClassName, int nClassID)
	{
		m_nStudentID = nStudentID;
		m_nClassID = nClassID;
		if (szStudentName != nullptr)
			strcpy(m_szStudentName, szStudentName);
		if (szClassName != nullptr)
			strcpy(m_szClassName, szClassName);
	}
	int m_nStudentID; // 学生ID
	char m_szStudentName[MAX_PATH]; // 学生姓名
	char m_szClassName[MAX_PATH]; // 班级名称
	int m_nClassID; // 班级ID
};

struct PkgS2CStudent:public PacketHeader
{
	PkgS2CStudent():PacketHeader(S2C_STUDENT, RESPONSEPKG, sizeof(PkgS2CStudent)-sizeof(PacketHeader))
	{}
	void Push(const StudentInfo& info)
	{
		m_stuInfo[m_nCount++] = info;
		m_length += sizeof(StudentInfo);
	}

	int m_nCount = 0;			//学生数量
	StudentInfo m_stuInfo[0];	// 学生信息
};
struct ClassInfo
{
	ClassInfo() = default;
	ClassInfo(int nClassID, const char* szClassName, int nStudentCountID)
	{
		m_nClassID = nClassID;
		if (szClassName != nullptr)
			strcpy(m_szClassName, szClassName);
		m_nStudentCount = nStudentCountID;
	}
	int m_nClassID; // 学生ID
	char m_szClassName[MAX_PATH]; // 班级名称
	int m_nStudentCount; // 班级ID
};
struct EnrollInfo
{

	EnrollInfo() = default;
	EnrollInfo(char* szStudentName,const char* szStudentId,const char* szClassName,
		const char* szCourseName, const char* szCourseID, const char* szScore)
	{
		if (szStudentName != nullptr)
			strcpy(m_szStudentName, szStudentName);
		if (szStudentId != nullptr)
			strcpy(m_szStudentID, szStudentId);
		if (szClassName != nullptr)
			strcpy(m_szClassName, szClassName);
		if (szCourseName != nullptr)
			strcpy(m_szCourseName, szCourseName);
		if (szCourseID != nullptr)
			strcpy(m_szCourseID, szCourseID);
		if (szScore != nullptr)
			strcpy(m_szScore, szScore);
	}
	char   m_szStudentName[MAX_PATH]; // 学生姓名
	char   m_szStudentID[MAX_PATH];              // 学生ID
	char   m_szClassName[MAX_PATH];   // 班级名称
	char   m_szCourseName[MAX_PATH];  // 课程名称
	char   m_szCourseID[MAX_PATH];               // 课程ID
	char   m_szScore[MAX_PATH];                  // 成绩
};
struct PkgS2CClass:public PacketHeader
{
	PkgS2CClass() :PacketHeader(S2C_CLASS, RESPONSEPKG, sizeof(PkgS2CClass) - sizeof(PacketHeader)) {}
	void Push(const  ClassInfo& classinfo) {
		m_classInfo[m_nCount++] = classinfo;
		m_length += sizeof(ClassInfo);
	}

	int m_nCount = 0;			//学生数量
	ClassInfo m_classInfo[0];	// 班级信息
};
struct PkgS2CEnroll :public PacketHeader
{
	PkgS2CEnroll() :PacketHeader(S2C_ENROLL, RESPONSEPKG, sizeof(PkgS2CEnroll) - sizeof(PacketHeader)) {}
	void Push(const EnrollInfo& enrollinfo)
	{
		m_enrollInfo[m_nCount++] = enrollinfo;
		m_length += sizeof(EnrollInfo);
	}
	int m_nCount = 0;			//学生数量
	EnrollInfo m_enrollInfo[0];	// 选课信息
};
struct PascalString
{
	int m_nLength;//长度
	char m_szStr[MAX_PATH];
	PascalString() : m_nLength(0) {
		memset(m_szStr, 0, sizeof(m_szStr));
	}
	PascalString(const char* str)
	{
		if (str != nullptr)
		{
			Init(str);
		}
	}
	void Init(const char* str) {
		if (str) {
			m_nLength = strlen(str);
			if (m_nLength > MAX_PATH - 1) {
				m_nLength = MAX_PATH - 1;
			}
			memcpy(m_szStr, str, m_nLength);
			m_szStr[m_nLength] = '\0'; // 确保字符串以null结尾
		}
		else {
			m_nLength = 0;
			memset(m_szStr, 0, sizeof(m_szStr));
		}
	}
};
struct CombineInfo{
	CombineInfo() = default;
	CombineInfo(const char* szClassName, const char* szClassId,
		const char* szCourseName, const char* szCourseId,
		const char* szStudentName, const char* szStudentId, const char* szScore)
	{
		if (szClassName != nullptr)
			m_szClassName.Init(szClassName);
		if (szClassId != nullptr)
			m_szClassId.Init(szClassId);
		if (szCourseName != nullptr)
			m_szCourseName.Init(szCourseName);
		if (szCourseId != nullptr)
			m_szCourseId.Init(szCourseId);
		if (szStudentName != nullptr)
			m_szStudentName.Init(szStudentName);
		if (szStudentId != nullptr)
			m_szStudentId.Init(szStudentId);
		if (szScore != nullptr)
			m_szScore.Init(szScore);
	}
	PascalString m_szClassName; // 班级名称
	PascalString m_szClassId;	// 班级ID
	PascalString m_szCourseName; // 课程名称
	PascalString m_szCourseId;	// 课程ID
	PascalString m_szStudentName; // 学生姓名
	PascalString m_szStudentId;	// 学生ID
	PascalString m_szScore;		// 分数
};
struct PkgS2CCombine:public PacketHeader
{
	PkgS2CCombine() :PacketHeader(S2C_COMBINE, RESPONSEPKG, sizeof(PkgS2CCombine) - sizeof(PacketHeader)) {}
	void Push(const CombineInfo& info)
	{
		m_combineInfo[m_nCount++] = info;
		m_length += sizeof(CombineInfo);
	}
	/*eg:SELECT
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
		WHERE
		s.StudentID IS NOT NULL
		-- AND c.ClassName = '计算机科学1班'  --按班级筛选（可选）
		-- AND co.CourseName = '数据结构'     --按课程筛选（可选）
		-- AND s.StudentName LIKE '%张%'      --按学生姓名筛选（可选）
		ORDER BY
		sc.Score ASC;  --按成绩升序（最低到最高）*/
	int m_nCount = 0;			//学生数量
	CombineInfo m_combineInfo[0];	// 选课信息
};
struct CourseInfo
{
	CourseInfo() = default;
	CourseInfo(int nCourseID, const char* szCourseName,int nStudentCount)
	{
		m_nCourseID = nCourseID;
		if (szCourseName != nullptr)
			strcpy(m_szCourseName, szCourseName);
		m_nStudentCount = nStudentCount;
	}
	int m_nCourseID; // 课程ID
	char m_szCourseName[MAX_PATH]; // 课程名称
	int m_nStudentCount;
};
struct PkgS2CCourse :public PacketHeader
{
	PkgS2CCourse() :PacketHeader(S2C_COURSE, RESPONSEPKG, sizeof(PkgS2CCourse) - sizeof(PacketHeader)) {}
	void Push(const CourseInfo& info)
	{
		m_courseInfo[m_nCount++] = info;
		m_length += sizeof(CourseInfo);
	}
	int m_nCount = 0;			//学生数量
	CourseInfo m_courseInfo[0];	// 学生信息
};

struct ClientStudentQueryPacket :public PacketHeader {
	ClientStudentQueryPacket() {
		m_PkgType = C2S_STUDENT;
		m_op = QUERYPKG;
		m_length = sizeof(ClientStudentQueryPacket);
	}
	PascalString m_szStuName;//学生姓名
	PascalString m_szCourseName;//课程名字
};
struct ClientCourseQueryPacket:public PacketHeader{
	ClientCourseQueryPacket() {
		m_PkgType = C2S_COURSE;
		m_op = QUERYPKG;
		m_length = sizeof(ClientCourseQueryPacket);
	}
	PascalString m_szCourseName; // 课程名称
};
struct StuQueryResult:public PacketHeader {
	StuQueryResult() {
		m_PkgType = S2C_STUDENT;
		m_op = RESPONSEPKG;
		m_length = sizeof(StuQueryResult);
	}
	bool m_success{ false }; // 查询是否成功
	PascalString m_errorMessage; // 错误信息
	int m_nRows{ 0 }; // 查询结果行数
	int m_nColumns{ 0 }; // 查询结果列数
	//跟着查询结果
};
	struct StudentDelete:public PacketHeader {
		StudentDelete() {
			m_PkgType = C2S_STUDENT;
			m_op = DELETEPKG;
			m_length = sizeof(StudentDelete);
		}
		int m_nStuID{ 0 }; // 学生ID
	};
struct ClassDelete :public PacketHeader {
	ClassDelete() {
		m_PkgType = C2S_CLASS;
		m_op = DELETEPKG;
		m_length = sizeof(ClassDelete);
	}
	int m_nClassID{ 0 }; // 班级ID
};
struct CourseDelete :public PacketHeader {
	CourseDelete() {
		m_PkgType = C2S_COURSE;
		m_op = DELETEPKG;
		m_length = sizeof(CourseDelete);
	}
	int m_nCourseID{ 0 }; // 课程ID
};
struct EnrollDelete:public PacketHeader
{
	EnrollDelete(){
		m_PkgType = C2S_ENROLL;
		m_op = DELETEPKG;
		m_length = sizeof(EnrollDelete);
	}
	//eg:DELETE FROM `school`.`enroll` WHERE `StudentID`=1 AND `CourseID`=2;
	PascalString m_szStudentId; // 学生ID
	PascalString m_szCourseId; // 课程ID
};
struct StudentAdd:public PacketHeader {
	StudentAdd() {
		m_PkgType = C2S_STUDENT;
		m_op = INSERTPKG;
		m_length = sizeof(StudentAdd);
	}
	//eg:INSERT INTO `school`.`student` (`StudentName`, `ClassID`) VALUES ('孙八', 3);
	//主键自增

	PascalString m_szStuName; // 学生姓名
	int m_CourseID{ 0 }; // 课程ID
};
struct ClassAdd :public PacketHeader {
	ClassAdd() {
		m_PkgType = C2S_CLASS;
		m_op = INSERTPKG;
		m_length = sizeof(ClassAdd);
	}
	//eg:INSERT INTO `school`.`class` (`ClassName`) VALUES ('三年级一班');
	PascalString m_szClassName; // 班级名称
};
struct CourseAdd :public PacketHeader {
	CourseAdd() {
		m_PkgType = C2S_COURSE;
		m_op = INSERTPKG;
		m_length = sizeof(CourseAdd);
	}
	//eg:INSERT INTO `school`.`course` (`CourseName`) VALUES ('数学');
	PascalString m_szCourseName; // 课程名称
};
struct  StudentUpdate :public StudentAdd {
	StudentUpdate() {
		m_PkgType = C2S_STUDENT;
		m_op = UPDATEPKG;
		m_length = sizeof(StudentUpdate);
	}
	//eg:UPDATE `school`.`student` SET `StudentName`='孙八' ,'ClassID'=3 WHERE `StudentID`=1;
	int m_nStuID{ 0 }; // 学生ID-pk
};
struct ClassUpdate:public PacketHeader
{
	ClassUpdate() {
		m_PkgType = C2S_CLASS;
		m_op = UPDATEPKG;
		m_length = sizeof(ClassUpdate);
	}
	int m_nClassID = 0;
	PascalString m_ClassName;
};
struct CourseUpdate :public PacketHeader {
	CourseUpdate() {
		m_PkgType = C2S_COURSE;
		m_op = UPDATEPKG;
		m_length = sizeof(CourseUpdate);
	}
	int m_nCourseID{ 0 }; // 课程ID
	PascalString m_szCourseName; // 课程名称
};
struct EnrollInsert :public PacketHeader {
	EnrollInsert() {
		m_PkgType = C2S_ENROLL;
		m_op = INSERTPKG;
		m_length = sizeof(EnrollInsert);
	}
	//eg:INSERT INTO `school`.`enroll` (`StudentID`, `CourseID`, `Score`) VALUES (1, 2, '90');
	PascalString m_szStudentId; // 学生ID
	PascalString m_szCourseId; // 课程ID
	PascalString m_szScore; // 成绩
};
struct EnrollUpdate :public PacketHeader {
	EnrollUpdate() {
		m_PkgType = C2S_ENROLL;
		m_op = UPDATEPKG;
		m_length = sizeof(EnrollUpdate);
	}
	//eg:UPDATE `school`.`enroll` SET `Score`='90' WHERE `StudentID`=1 AND `CourseID`=2;
	PascalString m_szStudentId; // 学生ID
	PascalString m_szCourseId; // 课程ID
	PascalString m_szScore; // 成绩
};
struct ClassQueryPacket:public PacketHeader {
	ClassQueryPacket() {
		m_PkgType = C2S_CLASS;
		m_op = QUERYPKG;
		m_length = sizeof(ClassQueryPacket);
	}
	PascalString m_szClassName; // 班级名称
};
struct EnrollQueryPacket :public PacketHeader {
	EnrollQueryPacket() {
		m_PkgType = C2S_ENROLL;
		m_op = QUERYPKG;
		m_length = sizeof(EnrollQueryPacket);
	}
	PascalString m_szStuName; // 学生姓名
	PascalString m_szCourseName; // 课程名称
};
struct ClassQueryResult :public PacketHeader {
	ClassQueryResult() {
		m_PkgType = S2C_CLASS;
		m_op = RESPONSEPKG;
		m_length = sizeof(ClassQueryResult);
	}
	bool m_success{ false }; // 查询是否成功
	PascalString m_errorMessage; // 错误信息
	int m_nRows{ 0 }; // 查询结果行数
	int m_nColumns{ 0 }; // 查询结果列数
	//跟着查询结果
};
struct CombineQueryPacket :public PacketHeader {
	CombineQueryPacket() {
		m_PkgType = C2S_COMBINE;
		m_op = QUERYPKG;
		m_length = sizeof(CombineQueryPacket);
	}
	PascalString m_szClassName; // 班级名称
	PascalString m_szCourseName; // 课程名称
	PascalString m_szStuName; // 学生姓名
	PascalString m_szLowestScore; // 最低分
	PascalString m_szHighestScore; // 最高分
	
	
};
struct StudentClear:public PacketHeader {
	StudentClear() {
		m_PkgType = C2S_COMBINE;
		m_op = CLEAR;
		m_length = sizeof(StudentClear);
	}
};
template<typename T>///T=ClientStudentQueryPacket _packet
inline void RecvRestPack(T& _packet, const PacketHeader& header) {
	memcpy(&_packet, &header, sizeof(PacketHeader));
	KCP::GetInstance().Recv(
		reinterpret_cast<char*>(&_packet) + sizeof(PacketHeader),
		sizeof(T) - sizeof(PacketHeader)
	);
}
#pragma pack(pop)