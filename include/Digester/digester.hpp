#ifndef DIGESTER_HPP
#define DIGESTER_HPP

#include "../ntHash/nthash.hpp"
#include <stdexcept>
#include <deque>

namespace digest{

// Only supports characters in DNA

class NotRolledException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Must call roll_one() or roll_next_minimizer() first.";
    }
};

class BadConstructionException : public std::exception
{
	const char * what () const throw ()
    {
    	return "minimized_h must be either 0, 1, or 2, k cannot be 0 or be greater than the length of the sequence, and pos can't be greater than the length of the sequence minus k";
    }
};

class BadSequenceLengthException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Sequence length must be greater than 0.";
    }
};

class NotRolledTillEndException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Iterator must be at the end of the current sequence before appending a new one.";
    }
};

// add more exceptions, seq.len >= k, minimized_h = 1, 2, or 3

// Only supports characters in DNA and N, upper or lower case
class Digester{
    public:
        /**
         * Constructor.
         * @param seq std string of DNA sequence to be hashed.
         * @param k K-mer size.
         * @param pos 0-indexed position in seq to start hashing from.
         * @param minimized_h hash to be minimized, 0 for canoncial, 1 for forward, 2 for reverse
         */
        Digester(const std::string& seq, unsigned k, size_t pos = 0, unsigned minimized_h = 0) 
            : seq(seq.data()), len(seq.size()), pos(pos), start(pos), end(pos+k), k(k), minimized_h(minimized_h) {
                this->c_outs = new std::deque<char>;
                if(k == 0 || pos > seq.size()-k || minimized_h > 2){
                    throw BadConstructionException();
                }
            }

        /**
         * Constructor.
         * @param seq C string of DNA sequence to be hashed.
         * @param len Length of seq.
         * @param k K-mer size.
         * @param start Position in seq to start hashing from.
         * @param minimized_h hash to be minimized, 0 for canoncial, 1 for forward, 2 for reverse
         */
        Digester(const char* seq, size_t len, unsigned k, size_t pos = 0, unsigned minimized_h = 0) 
            : seq(seq), len(len), pos(pos), start(pos), end(pos+k), k(k), minimized_h(minimized_h) {
                this->c_outs = new std::deque<char>;
                if(k == 0 || pos > len-k || minimized_h > 2){
                    throw BadConstructionException();
                }
            }

        virtual ~Digester(){
            delete c_outs;
        }
        
        /**
         * roll the hash 1 position to the right or construcuts the initial hash on first call 
         * 
         */
        void roll_one();


        /**
         * roll hash until we get to a minimizer or reach the end of the sequence
         * 
         * @return bool if a minimizer is found or exists, false if we reach end of seq before there is a minimizer
         */
        virtual bool roll_next_minimizer() = 0;

        // Possibly write another function that returns a group of minimizers instead of just rolling to the next one, 
        // TODO
        std::vector<size_t> roll_next_n_minis();

        size_t get_pos(){
            return pos;
        }

        uint64_t get_chash(){
            if(!rolled){
                throw NotRolledException();
            }
            return chash;
        }

        uint64_t get_fhash(){
            if(!rolled){
                throw NotRolledException();
            }
            return fhash;
        }

        uint64_t get_rhash(){
            if(!rolled){
                throw NotRolledException();
            }
            return rhash;
        }
        
        /*
            clear the deque
        */
        void new_seq(const std::string& seq, size_t pos){
            c_outs->clear();
            this->seq = seq.data();
            this->len = len;
            this->pos = pos;
            this->start = pos;
            this->end = pos+this->k;
            rolled = false;

            if(pos > seq.size()-k || minimized_h > 2){
                throw BadConstructionException();
            }
        }

        void new_seq(const char* seq, size_t len, size_t pos){
            c_outs->clear();
            this->seq = seq;
            this->len = len;
            this->pos = pos;
            this->start = pos;
            this->end = pos+this->k;
            rolled = false;
            if(pos > len-k || minimized_h > 2){
                throw BadConstructionException();
            }
        }

        /**
         * @param seq std string of DNA sequence to be appended
         * 
         * Simulates the appending of a new sequence to the end of the old sequence
         * The old string will no longer be stored, but the rolling hashes will be able to preceed as if the strings were appended
         * Cannot be called if roll_one hasn't been called at least once
         */
        void append_seq(const std::string& seq);

        /**
         * @param seq C string of DNA sequence to be appended
         * 
         * Simulates the appending of a new sequence to the end of the old sequence
         * The old string will no longer be stored, but the rolling hashes will be able to preceed as if the strings were appended
         * Cannot be called if roll_one hasn't been called at least once
         */
        void append_seq(const char* seq, size_t len);

        unsigned get_minimized_h(){
            return minimized_h;
        }

        
        /*
            if deque size is 0, then just read from start to end
            if deque size is not 0, read from left to right everything in deque, and then from 0 to end everything in seq
            assumes start is 0 after change string, even if it is irrelevant while the deque size isnt 0
        */
        std::string get_string();

        

    protected:
        // sequence to be digested
        const char* seq;
        
        // length of seq
        size_t len;
        
        // pos within entirety of the sequence you are digesting, sequences that are appended by append_seq are counted as one sequence
        size_t pos;
        
        // internal index of the next character to be thrown out, junk if c_outs is not empty
        size_t start;
        
        // internal index of next character to be added
        size_t end;
        
        // canonical hash
        uint64_t chash;
        
        // forward hash
        uint64_t fhash;
        
        // reverse hash
        uint64_t rhash;
        
        // length of kmer
        unsigned k;
        
        // deque of characters to be thrown out from left to right
        std::deque<char>* c_outs;

    private:
        /*
            Hash value to be minimized
            0 for canonical, 1 for forward, 2 for reverse
        */
        unsigned minimized_h;

        // internal bool to track if rolled was called at least once
        bool rolled = false;
};

}

#endif
