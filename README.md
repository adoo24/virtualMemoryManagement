# virtualMemoryManagement
1) Virtual Memory Simulator 인자
memsim simType firstLevelBits PhysicalMemorySizeBits TraceFileNames...... 
- 첫 번째 (simType) : 값 0, 1, 2, 3
0 일 때 : One-level page table system을 수행 합니다. FIFO Replacement를 사용합니다. 
1 일 때 : One-level page table system을 수행 합니다. LRU Replacement를 사용합니다. 
2 일 때 : Two-level page table system을 수행 합니다. LRU Replacement를 사용합니다. 
3 일 때 : Inverted page table system을 수행 합니다. LRU Replacement를 사용합니다. 
- 두 번째 (firstLevelBits - 1st level 주소 bits 크기) :
Two-level page table system을 수행할 때 사용되는 인자입니다. first level page table에 접근에 사용되는 메모리 주소 bits 의 수. 예를 들어 인자 값이 8 이면
Virtual address 32 bits 중에 앞의 8 bits 가 first level page table의 접근에 사용되며 따라서 first level page table의 entry의 수는 2
8 이 됩니다. Virtual address 가 32bits 이고
page 사이즈가 4Kbytes(12bits) 로 고정 되어 있기 때문에 firstLevelBits 인자가 결정되면 자동적으로 second level page table에 접근에 사용되는 주소 bits의 수가 결정됩니다. 위의 예에서
second level의 주소 bits 수는 32(virtual address bits)-12(page size bits)-8 ( first level 주소 bits 크기)로 계산 됩니다.
- 세 번째 (PhysicalMemorySizeBits - Physical Memory 크기 bits) :
Physical Memory의 크기를 나타내는 인자입니다. 예를 들어 인자 값이 n이면 Physical
Memory의 크기는 2n bytes 가 됩니다. 이 인자의 값으로는 32 보다 더 큰 값을 지원해도 되지만 최소한 12부터 32 bits 까지 반드시 지원 하여야 합니다. 예를 들어 해당 값이 12이면
Physical Memory의 크기는 4Kbytes이고 하나의 Frame으로 구성되었음을 말합니다. 
- 네 번째 및 그 이후의 인자들 (TraceFileNames, .... ) :
네 번째부터 그 이후의 인자 수는 제한이 없습니다. 이들 인자로는 메모리 trace를 저장한 파일의 이름이 나옵니다. mtraces 디렉토리에는 gcc, bzip, sixpack, swim의 실제 수행 프로그램에서 얻은 메모리 trace와 random 하게 만들어진 random0, random2 trace 파일이 있고 이들
각각의 파일에는 100만 개의 memory trace가 들어 있습니다. 시뮬레이션에서는 각각의 trace 파일에 있는 메모리 trace는 각각의 다른 프로세서가 수행하며
메모리를 접근한다고 가정합니다. 예를 들어 네 번째 인자로 “gcc.trace bzip.trace” 의 두 파일이 명시되면 process 0는 gcc.trace파일에 있는 메모리 trace의 메모리 주소를 차례대로 접근하게 되고 process 1 은 bzip.trace 파일에 있는 메모리 trace의 메모리 주소를 차례대로 접근한다고 가정 합니다. 따라서 네 번째 인자와 그 이후의 인자의 수만큼의 (명시된 trace 파일의
수) process 가 수행 된다 가정 합니다. 네 번째 인자와 그 이후의 인자로 나오는 trace 파일은 process 0부터 시작하여 process 1,
2, 3 ... 식으로 차례대로 할당됩니다. 각 process는 process 0부터 수행하는데 한번 메모리 접근을 하면 다음 process가 수행하여 메모리 접근을 하는 식으로 번갈아 차례대로 메모리에 접근하며 수행된다고 가정합니다.
