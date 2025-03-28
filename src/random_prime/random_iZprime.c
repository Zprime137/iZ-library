// random_iZprime.c
#include <iZ.h>

#include <random_iZprime.h> // For log_debug
#include <signal.h>         // For kill
#include <sys/wait.h>       // For waitpid

// random_iZprime is an algorithm for generating random very big primes

// Generate random base value for p which by incrementing by vx can yield primes
void set_random_base(mpz_t p, int matrix_id, mpz_t vx)
{
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_seed_randstate(state); // seed random state

    // create mpz_t tmp variable to hold values
    mpz_t tmp;
    mpz_init(tmp);

    // tmp = random_x in vx range
    mpz_urandomm(tmp, state, vx);

    // initialize p with the random_x value tmp
    iZ_gmp(p, tmp, matrix_id);

    // find next x co-prime with vx
    for (int i = 1; i < 10000; i++)
    {
        // increment p by 6 to increment x by 1
        mpz_add_ui(p, p, 6);

        // Compute tmp = gcd(vx, p)
        mpz_gcd(tmp, vx, p);

        // break if tmp = 1,
        // implying current x value can yield primes of the form iZ(x + vx * y, matrix_id)
        if (mpz_cmp_ui(tmp, 1) == 0)
            break;
    }

    // set p = iZ(random_x + vx, matrix_id) to skip first row in iZm
    mpz_add(p, p, vx);

    // cleaning up
    mpz_clear(tmp);
    gmp_randclear(state);
}

// Search for a random prime in the iZ-Matrix
void search_p_in_iZm(mpz_t p, int matrix_id, mpz_t vx, int primality_check_rounds)
{
    mpz_t tmp;
    mpz_init(tmp);

    // set random x and y values for p = iZ(x + vx * y),
    // such that p and vx are co-primes
    set_random_base(tmp, matrix_id, vx);
    mpz_set_ui(p, 0);

    int attempts_limit = 1000000;

    for (int i = 0; i < attempts_limit; i++)
    {
        mpz_add(tmp, tmp, vx); // increment y

        // break if tmp is prime and set p = tmp
        if (mpz_probab_prime_p(tmp, primality_check_rounds))
        {
            mpz_set(p, tmp);
            break;
        }
    }

    if (mpz_cmp_ui(p, 0) == 0)
    {
        log_debug("No Prime Found :\\\n");
        search_p_in_iZm(p, matrix_id, vx, primality_check_rounds);
    }

    mpz_clear(tmp);
}

// Random iZprime algorithm: Entry point
void random_iZprime(mpz_t p, int p_id, int bit_size, int primality_check_rounds, int cores_num)
{
    // 1. Compute vx for the given bit-size
    mpz_t vx;
    mpz_init(vx);
    gmp_compute_max_vx(vx, bit_size);

    // 2. If only one core, run the search in-process
    if (cores_num < 2)
    {
        search_p_in_iZm(p, p_id, vx, primality_check_rounds);
        return;
    }

    // 3. Else, fork multiple processes to search for a prime
    // Create a pipe for inter-process communication.
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pids[cores_num];

    // Fork child processes.
    for (int i = 0; i < cores_num; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child process: close the read-end.
            mpz_t local_candidate;
            mpz_init(local_candidate);

            // Search for a candidate prime.
            search_p_in_iZm(local_candidate, p_id, vx, primality_check_rounds);

            // If a candidate is found, send it via the pipe.
            if (mpz_cmp_ui(local_candidate, 0) != 0)
            {
                char *candidate_str = mpz_get_str(NULL, 10, local_candidate);
                if (candidate_str != NULL)
                {
                    write(fd[1], candidate_str, strlen(candidate_str) + 1);
                    free(candidate_str);
                }
            }
            mpz_clear(local_candidate);
            close(fd[1]);
            exit(0);
        }
        else
        {
            // Parent process saves child's PID.
            pids[i] = pid;
        }
    }

    // 4. Parent reads first result from the pipe.
    close(fd[1]);
    int str_size = bit_size / 3;
    char buf[str_size];
    memset(buf, 0, str_size);

    ssize_t n = read(fd[0], buf, str_size);
    if (n > 0)
    {
        if (mpz_set_str(p, buf, 10) != 0)
            fprintf(stderr, "Error converting string to mpz_t\n");
    }
    close(fd[0]);

    // 5. Terminate all child processes
    for (int i = 0; i < cores_num; i++)
    {
        kill(pids[i], SIGTERM);    // Terminate child process
        waitpid(pids[i], NULL, 0); // Wait for child process to terminate
    }

    // Cleanup
    mpz_clear(vx);
}
