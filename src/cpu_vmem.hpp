#pragma once
#include <fstream>

inline std::uint64_t get_vsize(
		const std::uint32_t pid_
		) {
	if (pid_ == 0) {
		return 0;
	}

	std::ifstream ifs("/proc/" + std::to_string(pid_) + "/stat");
	if (!ifs) {
		return 0;
	}

	std::string pid, comm, state, ppid, pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, cmajflt, utime, stime, cutime, cstime, priority, nice, O, itrealvalue, starttime;

	std::uint64_t vsize;

	ifs >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
			>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
			>> utime >> stime >> cutime >> cstime >> priority >> nice
			>> O >> itrealvalue >> starttime >> vsize;

	ifs.close();

	return vsize;
}
