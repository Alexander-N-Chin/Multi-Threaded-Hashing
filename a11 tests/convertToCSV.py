import re
import csv
class Run:
    time = 0
    threads_per_block = 0
    no_of_blocks = 0
    hash_value = 0
    no_threads = 0

    def __init__(self, result):
        self.time = re.search(r"time taken = (\d+.?\d*)", result).group(1)
        self.threads_per_block = re.search(r"no\. of threads per block = (\d+)", result).group(1)
        self.no_of_blocks = re.search(r"no\. of blocks = (\d+)", result).group(1)
        self.hash_value = re.search(r"hash value = (\d+)", result).group(1)
        self.no_threads = re.search(r"tc\d (\d+)", result).group(1)

    def __str__(self):
        return f"""no. of blocks: {self.no_of_blocks}
no. of threads: {self.no_threads}
blocks_per_thread = {self.threads_per_block}
hash value: {self.hash_value}
time taken: {self.time}\n"""

def main():
    testcase = "tc2"
    testname = "test2"
    f = open(f"{testname}_{testcase}_1000.txt", "r")
    content = f.read()
    runs = content.split("\n\n")
    Runs = []
    for run in runs:
        match = re.search(r"no\. of blocks = (\d+)", run)
        Runs.append(Run(run))

    # Define the structure of the data
    student_header = ['no. of blocks', 'no. of threads', 'no. blocks per threads', 'hash value', 'time taken']
    # Define the actual data
    data = []
    for run in Runs:
        data.append([run.no_of_blocks, run.no_threads, run.threads_per_block, run.hash_value, run.time])
    # 1. Open a new CSV file
    with open(f'{testname}_{testcase}_1000.csv', 'w') as file:
        # 2. Create a CSV writer
        writer = csv.writer(file)
        # 3. Write data to the file
        writer.writerow(student_header)
        writer.writerows(data)

if __name__ == "__main__":
    main()