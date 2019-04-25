//   Copyright 2019 <Huawei Technologies Co., Ltd>
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "simulator-mpi/funcs.hpp"
#include "simulator-mpi/SwapperMT.hpp"
#include "simulator-mpi/SimulatorMPI.hpp"

#include <boost/mpi.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <complex>
//#include <omp.h>

#include <glog/logging.h>

#include "simulator-mpi/swapping.hpp"

using std::cout;
using std::endl;

namespace po = boost::program_options;

template<class T>
std::vector<T> allocateData(int rank, uint64_t M) {
	std::vector<T> res(1ul << M);
	for(size_t i = 0; i < res.size(); ++i)
		res[i] = std::complex<double>(rank, i);

	return res;
}


SimulatorMPI::StateVector allocateData1(int rank, uint64_t M) {
	SimulatorMPI::StateVector res(1ul << M);
	for(size_t i = 0; i < res.size(); ++i)
		res[i] = rank*(1ul << M) +  i;

	return res;
}

typedef typename SwapperMT::value_type value_type;


int main(int argc, const char** argv) {

    google::InitGoogleLogging("qswap_mpi");

	uint64_t L = 0;
	uint64_t M = 0;
	uint64_t nthreads = 1;
	uint64_t rank = 0;
	std::vector<uint64_t> swap_pairs;
	std::vector<uint64_t> perm_pairs;

	po::options_description desc("Options");
	desc.add_options()
			("help", "produce help message")
			("L", po::value<uint64_t>(&L), "total number of qubits")
			("M", po::value<uint64_t>(&M), "number of local qubits")
			("nthreads", po::value<uint64_t>(&nthreads), "number of OMP threads")
			("rank", po::value<uint64_t>(&rank), "rank")
			("swap", po::value<std::vector<uint64_t>>(&swap_pairs)->multitoken(), "pair to swap")
			("perm", po::value<std::vector<uint64_t>>(&perm_pairs)->multitoken(), "initial permutation by pairs")
	  ;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
	    cout << desc << "\n";
	    return 0;
	}

//	omp_set_num_threads(nthreads);

	mpi::environment env;
	mpi::communicator world;

    rank = world.rank();

	LOG(INFO) << "I am process " << rank << " of " << world.size()
			<< "." << std::endl;

	LOG(INFO) << format("L: %d, M: %d") % L % M << endl;

    LOG(INFO) << format("Current permutation: %s") % printPairs(perm_pairs);
    LOG(INFO) << format("Do swaps: %s") % printPairs(swap_pairs);

    LOG(INFO) << format("R: %d, Allocating %d local qubits...") % rank % M << endl;
	auto t0 = std::chrono::high_resolution_clock::now();
	auto state_vector = allocateData1(rank, M); //SimulatorMPI::StateVector
// 	auto state_vector = allocateData<uint64_t>(rank, M);
	auto t1 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> dt = t1-t0;
	LOG(INFO) << format("R: %d,  ...done, %.3f ms" ) % rank % dt.count() << endl;

     value_type res = 0;

//     auto perm = permuteQubits(naturalOrdering(L), perm_pairs);
// 	std::vector<uint64_t> swapBits = q2bits<uint64_t>(M, swap_pairs, inversePermutation(perm));
//     LOG(INFO) << format("Swap bits: %s") % printPairs(swapBits);
//
//     uint64_t color = getColor<uint64_t>(rank, swapBits);
//     auto comm = world.split(color);
//     LOG(INFO) << format("R: %d, color: %d, comm size: %d") % rank % color % comm.size()  << endl;
//
// //     auto s = Swapper<value_type>(state_vector, comm, rank, color, L, M, swapBits);
// //     res = s.doSwap();

//     SwapperMT s(world, state_vector, rank, M, comm.size());
//     s.doSwap(world.rank(), comm, color, swapBits);
//     LOG(INFO) << "local satte vector: " << endl << print(state_vector);

// 	auto t2 = std::chrono::high_resolution_clock::now();
// 	std::chrono::duration<double, std::milli> dt2 = t2-t1;
//     LOG(INFO) << format("swap time: %.3f ms") % dt2.count();
//     printStateVector(std::cerr, "Local state vector:", L, state_vector);

//     size_t free_idx_start = 0;
//     size_t maxSend = 3;
//     size_t n = maxSend + swapBits.size()/2; // n must be < state_vector.size()
//     SwapArrays<uint64_t> arrs( (1ul << n) );
//     free_idx_start = swapping::Swapping<5>::calcSwap(state_vector.begin(), free_idx_start, arrs, swapBits);

//     printStateVector(std::cerr, "svalues: ", L, arrs.svalues);

    world.barrier();

//     cout << "State vector:" << endl; printStateVector(state_vector);

    LOG(INFO) << format("R: %d, res: %d, finish!") % rank % res << endl;
    LOG(INFO) << endl << "================================" << endl << endl;

	return 0;
}