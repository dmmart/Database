#include <C:\PostgreSQL\include\libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LENGTH 100

int searchForStudent(PGconn *conn, unsigned long *studentCode);
void addStudent(PGconn *conn, unsigned long *studentCode);
void modifyStudent(PGconn *conn, unsigned long *studentCode);
void deleteStudent(PGconn *conn, unsigned long *studentCode);
int searchForCourse(PGconn *conn, char *subjectCode);
void addCourse(PGconn *conn, char *subjectCode);
void modifyCourse(PGconn *conn, char *subjectCode);
void deleteCourse(PGconn *conn, char *subjectCode);
//void assignGrade(PGconn *conn);
//void averageGradeSearch(PGconn *conn);

// additional functions
void generateStudentCode(PGconn *conn, unsigned long *studentCode);
char *readString();
void getDate(char *date);

int main(void)
{
    unsigned option;
    char subjectCode[8];

    PGconn *conn;
    conn = PQconnectdb("dbname=mveb176399 host=ewis.pld.ttu.ee user=mveb176399 password=lydvclC65qef12");

    if(PQstatus(conn) == CONNECTION_BAD)
    {
        printf("Bad connection\nCannot access database...\n");
        exit(1);
    }

    while(1)
    {
        int variant1, variant2;
        unsigned long studentCode = 0;

        printf("\n\n        Choose:\n"
               "1. Add student with random student code\n"
               "2. Search for student\n"
               "3. Search for course\n"
               "4. Assign grade to student\n"
               "5. Display students based on average grade\n"
               "0. Exit\n>");
        scanf("%u", &option);

        switch(option)
        {
        case 1:
            generateStudentCode(conn, &studentCode);
            addStudent(conn, &studentCode);
            break;
        case 2:
            variant1 = searchForStudent(conn, &studentCode);
            switch(variant1)
            {
            case 1:
                addStudent(conn, &studentCode);
                break;
            case 2:
                modifyStudent(conn, &studentCode);
                break;
            case 3:
                deleteStudent(conn, &studentCode);
                break;
            }
            break;
        case 3:
            variant2 = searchForCourse(conn, subjectCode);
            switch(variant2)
            {
            case 1:
                addCourse(conn, subjectCode);
                break;
            case 2:
                modifyCourse(conn, subjectCode);
                break;
            case 3:
                deleteCourse(conn, subjectCode);
                break;
            }
            break;
        case 4:
            //assignGrade(conn);
            break;
        case 5:
            //averageGradeSearch(conn);
            break;
        case 0:
            PQfinish(conn);
            return 0;
            break;
        default:
            printf("Select from 1 to 5 or 0...");
        }
    }
}



void generateStudentCode(PGconn *conn, unsigned long *studentCode)
{
    int i = 0, rows;

    PGresult *res = NULL;
    res = PQexec(conn, "SELECT * FROM student;");
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("No data received...\n");
        PQfinish(conn);
        exit(1);
    }
    rows = PQntuples(res);

    for(i = 0; i < rows; i++)
    {
        if(*studentCode < atoi(PQgetvalue(res, i, 0)))
        {
            *studentCode = atoi(PQgetvalue(res, i, 0));
        }
    }
    (*studentCode)++;

    PQclear(res);
}

int searchForStudent(PGconn *conn, unsigned long *studentCode)
{
    int var = 0, i, rows;
    char choice;

    PGresult *res = NULL;
    res = PQexec(conn, "SELECT * FROM student;");
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("No data received...\n");
        PQfinish(conn);
        exit(1);
    }
    rows = PQntuples(res);

    do
    {
        printf("Enter student code:\n>");
        scanf("%lu", studentCode);
    }
    while(*studentCode < 100000 || *studentCode > 999999);


    for(i = 0; i < rows; i++)
    {
        if(*studentCode == atoi(PQgetvalue(res, i, 0)))
        {
            var++;
            break;
        }
    }

    if(!var)
    {
        printf("No such student found\nDo You want to create student with such student code? (y / n)\n>");
        scanf(" %c", &choice);
        if(choice == 'y')
        {
            var = 1;
        }
    }
    else
    {
        printf("The student with %lu code is %s %s %s\n", *studentCode, PQgetvalue(res, i, 1), PQgetvalue(res, i, 2), PQgetvalue(res, i, 3));
        printf("Press 'm' to modify\nPress 'd' to delete\n(any letter to skip)\n>");
        scanf(" %c", &choice);

        switch(choice)
        {
        case 'm':
            var = 2;
            break;
        case 'd':
            var = 3;
            break;
        }
    }

    PQclear(res);
    return var;
}

void addStudent(PGconn *conn, unsigned long *studentCode)
{
    int i, rows;
    char *firstName, *lastName;
    char date[11];// yyyy-mm-dd
    char choice;
    char buffer[MAX_LENGTH];

    PGresult *res = NULL;
    res = PQexec(conn, "SELECT * FROM student;");
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("No data received...\n");
        PQfinish(conn);
        exit(1);
    }

    printf("Enter first name:\n>");
    firstName = readString();

    printf("Enter last name:\n>");
    lastName = readString();

    getDate(date);

    printf("Add %lu %s %s %s? (y / n)\n>", *studentCode, firstName, lastName, date);
    scanf(" %c", &choice);

    if(choice == 'y')
    {
        sprintf(buffer, "INSERT INTO student (student_code, first_name, last_name, birthdate) VALUES ('%lu','%s', '%s', '%s');", *studentCode, firstName, lastName, date);
        res = PQexec(conn, buffer);

        sprintf(buffer, "INSERT INTO grade (student_code) VALUE ('%lu');", *studentCode);
        res = PQexec(conn, buffer);

        sprintf(buffer, "SELECT student_code, first_name, last_name, birthdate, average_grade FROM student WHERE student_code = '%lu'", *studentCode);
        res = PQexec(conn, buffer);

        if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            printf("No data received...\n");
            PQfinish(conn);
            exit(1);
        }
        rows = PQntuples(res);

        for(i = 0; i < rows; i++)
        {
            if(*studentCode == atoi(PQgetvalue(res, i, 0)))
            {
                printf("Student added successfully\n");
            }
        }
    }

    PQclear(res);
    free(firstName);
    free(lastName);
}

void modifyStudent(PGconn *conn, unsigned long *studentCode)
{
    PGresult *res = NULL;
    int i, rows;
    char* firstName, *lastName;
    unsigned choice;
    char date[11];
    char buffer[MAX_LENGTH];
    unsigned long oldStudentCode;

    while(1)
    {
        printf("\n1. Change first name\n"
               "2. Change last name\n"
               "3. Change student code\n"
               "4. Change birth date\n"
               "0. Menu\n>");
        scanf("%u", &choice);

        switch(choice)
        {
        case 1:
            printf("Enter new name:\n>");
            firstName = readString();

            sprintf(buffer, "UPDATE student SET first_name = '%s' WHERE student_code = '%lu';", firstName, *studentCode);
            res = PQexec(conn, buffer);

            sprintf(buffer,"SELECT student_code, first_name, last_name, birthdate FROM student WHERE first_name = '%s' AND student_code = '%lu';",firstName, *studentCode);
            res = PQexec(conn, buffer);
            if(PQresultStatus(res) != PGRES_TUPLES_OK)
            {
                printf("No data received...\n");
                PQfinish(conn);
                exit(1);
            }
            rows = PQntuples(res);

            for(i = 0; i < rows; i++)
            {
                if(*studentCode == atoi(PQgetvalue(res, i, 0)))
                {
                    printf("Name changed successfully\n");
                }
            }
            break;
        case 2:
            printf("Enter new surname:\n>");
            lastName = readString();

            sprintf(buffer, "UPDATE student SET last_name = '%s' WHERE student_code = '%lu';", lastName, *studentCode);
            res = PQexec(conn, buffer);

            sprintf(buffer,"SELECT student_code, first_name, last_name, birthdate FROM student WHERE first_name = '%s' AND student_code = '%lu';", firstName, *studentCode);
            res = PQexec(conn, buffer);
            if(PQresultStatus(res) != PGRES_TUPLES_OK)
            {
                printf("No data received...\n");
                PQfinish(conn);
                exit(1);
            }
            rows = PQntuples(res);

            for(i = 0; i < rows; i++)
            {
                if(*studentCode == atoi(PQgetvalue(res, i, 0)))
                {
                    printf("Surname changed successfully\n");
                }
            }
            break;
        case 3:
            oldStudentCode = *studentCode;
            generateStudentCode(conn, studentCode);

            sprintf(buffer, "UPDATE student SET student_code = '%lu' WHERE student_code = '%lu';", *studentCode, oldStudentCode);
            res = PQexec(conn, buffer);

            sprintf(buffer,"SELECT student_code, first_name, last_name, birthdate FROM student WHERE  student_code = '%lu';", *studentCode);
            res = PQexec(conn, buffer);
            if(PQresultStatus(res) != PGRES_TUPLES_OK)
            {
                printf("No data received...\n");
                PQfinish(conn);
                exit(1);
            }
            rows = PQntuples(res);

            for(i = 0; i < rows; i++)
            {
                if(*studentCode == atoi(PQgetvalue(res, i, 0)))
                {
                    printf("Student code changed successfully\n");
                }
            }
            break;
        case 4:
            getDate(date);

            sprintf(buffer, "UPDATE student SET birthdate = '%s' WHERE student_code = '%lu';", date, *studentCode);
            res = PQexec(conn, buffer);

            sprintf(buffer,"SELECT student_code, first_name, last_name, birthdate FROM student WHERE birthdate = %s student_code = %lu;", date, *studentCode);
            res = PQexec(conn, buffer);
            if(PQresultStatus(res) != PGRES_TUPLES_OK)
            {
                printf("No data received...\n");
                PQfinish(conn);
                exit(1);
            }
            rows = PQntuples(res);

            for(i = 0; i < rows; i++)
            {
                if(*studentCode == atoi(PQgetvalue(res, i, 0)))
                {
                    printf("Birth date changed successfully\n");
                }
            }
            break;
        case 0:
            break;
        default:
            printf("Select from 1 to 4 or 0...\n");
        }
    }
    PQclear(res);
    free(firstName);
    free(lastName);
}

void deleteStudent(PGconn *conn, unsigned long *studentCode)
{
    PGresult *res = NULL;
    int i, rows;
    char buffer[MAX_LENGTH];

    sprintf(buffer, "DELETE FROM student WHERE student_code = '%lu';", *studentCode);
    res = PQexec(conn, buffer);
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("No data received...\n");
        PQfinish(conn);
        exit(1);
    }
    rows = PQntuples(res);


    for(i = 0; i < rows; i++)
    {
        if(*studentCode == atoi(PQgetvalue(res, i, 0)))
        {
            printf("Student was not deleted...");
            PQclear(res);
            exit(1);
        }
    }

    printf("Student deleted successfully\n");

    PQclear(res);
}

int searchForCourse(PGconn *conn, char *subjectCode)
{
    int var = 0, i, rows;
    char choice;

    PGresult *res = NULL;
    res = PQexec(conn, "SELECT * FROM subject;");
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("No data received...\n");
        PQfinish(conn);
        exit(1);
    }
    rows = PQntuples(res);

    printf("Enter subject code:\n>");
    scanf("%s", subjectCode);

    for(i = 0; i < rows; i++)
    {
        if(!strcmp(subjectCode, PQgetvalue(res, i, 2)))
        {
            var++;
            break;
        }
    }

    if(!var)
    {
        printf("No such subject found\nDo You want to create subject with such subject code? (y / n)\n>");
        scanf(" %c", &choice);
        if(choice == 'y')
        {
            var = 1;
        }
    }
    else
    {
        printf("The subject with %s code is %s\n", subjectCode, PQgetvalue(res, i, 3));
        printf("Press 'm' to modify\nPress 'd' to delete\n(any letter to skip)\n>");
        scanf(" %c", &choice);

        switch(choice)
        {
        case 'm':
            var = 2;
            break;
        case 'd':
            var = 3;
            break;
        }
    }

    PQclear(res);
    return var;
}

void addCourse(PGconn *conn, char *subjectCode)
{
    int i, rows, subjectID = 0, credits;
    char* teacherName, *subjectName;
    char choice;
    char buffer[MAX_LENGTH];

    PGresult *res = NULL;
    res = PQexec(conn, "SELECT * FROM subject;");
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("No data received...\n");
        PQfinish(conn);
        exit(1);
    }
    rows = PQntuples(res);

    printf("Enter subject name:\n>");
    subjectName = readString();

    printf("Enter teacher's name:\n>");
    teacherName = readString();

    printf("Enter number of credits:\n>");
    scanf("%d", &credits);

    for(i = 0; i < rows; i++)
    {
        if(subjectID < atoi(PQgetvalue(res, i, 0)))
        {
            subjectID = atoi(PQgetvalue(res, i, 0));
        }
    }
    subjectID++;

    printf("Add %s %s (%d credits) %s? (y / n)\n>", subjectCode, subjectName, credits, teacherName);
    scanf(" %c", &choice);

    if(choice == 'y')
    {
        sprintf(buffer, "INSERT INTO subject (id_subject, credits, subject_code, name, teacher) VALUES ('%d','%d', '%s', '%s', '%s');", subjectID, credits, subjectName, subjectCode, teacherName);
        res = PQexec(conn, buffer);

        if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            printf("No data received...\n");
            PQfinish(conn);
            exit(1);
        }
        rows = PQntuples(res);

        for(i = 0; i < rows; i++)
        {
            if(!strcmp(subjectCode, PQgetvalue(res, i, 0)))
            {
                printf("Course added successfully\n");
            }
        }
    }

    PQclear(res);
    free(subjectName);
    free(teacherName);
}

void modifyCourse(PGconn *conn, char *subjectCode)
{
    PGresult *res = NULL;
    int i, rows;
    char* teacherName, *subjectName;
    unsigned choice;
    char buffer[MAX_LENGTH];

    while(1)
    {
        printf("\n1. Change teacher name\n"
               "2. Change course name\n"
               "0. Menu\n>");
        scanf("%u", &choice);

        switch(choice)
        {
        case 1:
            printf("Enter new name:\n>");
            teacherName = readString();

            sprintf(buffer, "UPDATE subject SET teacher = '%s' WHERE subject_code = '%s';", teacherName, subjectCode);
            res = PQexec(conn, buffer);

            sprintf(buffer,"SELECT subject WHERE teacher = '%s' AND subject_code = '%s';", teacherName, subjectCode);
            res = PQexec(conn, buffer);
            if(PQresultStatus(res) != PGRES_TUPLES_OK)
            {
                printf("No data received...\n");
                PQfinish(conn);
                exit(1);
            }
            rows = PQntuples(res);

            for(i = 0; i < rows; i++)
            {
                if(!strcmp(subjectCode, PQgetvalue(res, i, 0)))
                {
                    printf("Name changed successfully\n");
                }
            }
            break;
        case 2:
            printf("Enter new subject name:\n>");
            subjectName = readString();

            sprintf(buffer, "UPDATE student SET subject_name = '%s' WHERE subject_code = '%s';", teacherName, subjectCode);
            res = PQexec(conn, buffer);

            sprintf(buffer,"SELECT subject WHERE subject_name = '%s' AND subject_code = '%s';", teacherName, subjectCode);
            res = PQexec(conn, buffer);
            if(PQresultStatus(res) != PGRES_TUPLES_OK)
            {
                printf("No data received...\n");
                PQfinish(conn);
                exit(1);
            }
            rows = PQntuples(res);

            for(i = 0; i < rows; i++)
            {
                if(!strcmp(subjectCode, PQgetvalue(res, i, 0)))
                {
                    printf("Subject name changed successfully\n");
                }
            }
            break;
        case 0:
            break;
        default:
            printf("Select from 1 to 2 or 0...\n");
        }
    }
    PQclear(res);
    free(subjectName);
    free(teacherName);
}

void deleteCourse(PGconn *conn, char *subjectCode)
{
    PGresult *res = NULL;
    int i, rows;
    char buffer[MAX_LENGTH];

    sprintf(buffer, "DELETE FROM subject WHERE subject_code = '%s';", subjectCode);
    res = PQexec(conn, buffer);
    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        printf("No data received...\n");
        PQfinish(conn);
        exit(1);
    }
    rows = PQntuples(res);

    for(i = 0; i < rows; i++)
    {
        if(!strcmp(subjectCode, PQgetvalue(res, i, 0)))
        {
            printf("Subject was not deleted...");
            PQclear(res);
            exit(1);
        }
    }

    printf("Student deleted successfully\n");

    PQclear(res);
}

/*void assignGrade(PGconn *conn)
{

}*/

char *readString()
{
    char *str;
    char buffer[MAX_LENGTH];
    scanf(" %[^\n]s", buffer);

    str = (char*)malloc(strlen(buffer)+1);
    if(str == NULL)
    {
        printf("Memory was not allocated...\n");
        exit(1);
    }
    strcpy(str, buffer);

    return str;
}

void getDate(char *date)
{
    int year, currentYear, month, day, wrongDate;

    time_t now = time(NULL);
    struct tm tm = *localtime(&now);

    currentYear = tm.tm_year + 1900;

    do
    {
        wrongDate = 0;

        printf("Enter date ( yyyy-mm-dd )\n>");
        scanf("%d-%d-%d", &year, &month, &day);

        if(year > currentYear || year < currentYear - 100)
        {
            wrongDate++;
        }
        if(month < 1 || month > 12)
        {
            wrongDate++;
        }
        if(day < 1)
        {
            wrongDate++;
        }
        switch(month)
        {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            if(day > 31)
            {
                wrongDate++;
            }
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            if(day > 30)
            {
                wrongDate++;
            }
            break;
        case 2:
            if(day > 29)
            {
                wrongDate++;
            }
            break;
        }
        if((((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) && day > 28)
        {
            wrongDate++;
        }
    }
    while(wrongDate);

    sprintf(date, "%d-%.2d-%.2d", year, month, day);
}
