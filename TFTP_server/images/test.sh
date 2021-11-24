#!/bin/bash
# dos2unix -n test.sh test1.sh

echo "Test1. 3CR3LF.txt"
tftp << 'END'
connect 127.0.0.1 8080
binary
status
get files_for_testing/3CR3LF.txt client_3CR3LF.txt
END
echo ""
echo "diff files_for_testing/3CR3LF.txt client_3CR3LF.txt"
diff files_for_testing/3CR3LF.txt client_3CR3LF.txt << 'END'
END

echo ""

echo "Test2. 3LF.txt"
tftp << 'END'
connect 127.0.0.1 8080
binary
status
get files_for_testing/3LF.txt client_3LF.txt
END
echo ""
echo "diff files_for_testing/3LF.txt client_3LF.txt"
diff files_for_testing/3LF.txt client_3LF.txt << 'END'
END

echo ""

echo "Test3. 34MB.txt"
tftp << 'END'
connect 127.0.0.1 8080
binary
status
get files_for_testing/34MB.txt client_34MB.txt
END
echo ""
echo "diff files_for_testing/34MB.txt client_34MB.txt"
diff files_for_testing/34MB.txt client_34MB.txt << 'END'
END

echo ""

echo "Test4. 2047bin"
tftp << 'END'
connect 127.0.0.1 8080
binary
status
get files_for_testing/2047bin client_2047bin
END
echo ""
echo "diff files_for_testing/2047bin client_2047bin"
diff files_for_testing/2047bin client_2047bin << 'END'
END

echo ""

echo "Test5. 2048bin"
tftp << 'END'
connect 127.0.0.1 8080
binary
status
get files_for_testing/2048bin client_2048bin
END
echo ""
echo "diff files_for_testing/2048bin client_2048bin"
diff files_for_testing/2048bin client_2048bin << 'END'
END

echo ""

echo "Test6. 34MBbin"
tftp << 'END'
connect 127.0.0.1 8080
binary
status
get files_for_testing/34MBbin client_34MBbin
END
echo ""
echo "diff files_for_testing/34MBbin client_34MBbin"
diff files_for_testing/34MBbin client_34MBbin << 'END'
END

echo ""