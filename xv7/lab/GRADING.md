# Grading

1. Download student submissions:

    ```bash
    mkdir -p submissions
    rsync -rtLP ${your_kerberos}@athena.dialup.mit.edu:/mit/6.828/web_scripts/$(date +"%Y")/uploads/lab-${lab_name}/ submissions/
    ```

2. Start the grading VM and SSH in:

    ```bash
    vagrant up
    vagrant ssh
    ```

3. Grade submissions (in the VM):

    ```bash
    cd /xv6
    ./batch-grade --config conf/lab-${lab_name}.json --output lab-${lab_name}.json submissions
    ```

4. Copy grades to Athena:

    ```bash
    rsync lab-${lab_name}.json ${your_kerberos}@athena.dialup.mit.edu:/mit/6.828/web_scripts/$(date +"%Y")/grades/
    ```

5. Import grades:

    ```bash
    ssh ${your_kerberos}@athena.dialup.mit.edu
    cd /mit/6.828/web_scripts/$(date +"%Y")/
    ./import-grades.py -l ${lab_name}
    ```
