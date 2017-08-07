
#ifndef STATISTIC_H
#define STATISTIC_H
#include<iostream>

struct statistic
{
	int total_send;
	int total_recv;

	int recv;

	int time_cost;
	int time_cost_min;
	int time_cost_max;

	int time_cost_average;
	int time_cost_df_max;


	statistic()
	{
		total_send = 0;
		total_recv = 0;

		recv = 0;

		time_cost = 0;
		time_cost_min = 999999;
		time_cost_max = 0;

		time_cost_average = 0;
		time_cost_df_max = 0;
	}
};


void calculate(statistic* stat)
{
	if (stat->recv == 0)
	{
		stat->time_cost_average = 0;
		stat->time_cost_df_max = 0;
	}
	else
	{
		stat->time_cost_average = stat->time_cost / stat->recv;
		stat->time_cost_df_max = stat->time_cost_max - stat->time_cost_min;
	}

	stat->recv = 0;
	stat->time_cost = 0;
	stat->time_cost_min = 999999;
	stat->time_cost_max = 0;
}

void on_send(statistic* stat)
{
	++stat->total_send;
}

void on_recv(statistic* stat, int time_cost)
{
	++stat->total_recv;

	++stat->recv;
	stat->time_cost += time_cost;
	stat->time_cost_min = stat->time_cost_min > time_cost ? time_cost : stat->time_cost_min;
	stat->time_cost_max = stat->time_cost_max > time_cost ? stat->time_cost_max : time_cost;
}

void output(const char* title,statistic* stat)
{
	std::cout << std::endl;
	std::cout << " ----- "<< title <<" --begin--------------- " << std::endl;
	std::cout << " total send: " << stat->total_send << " total recv:" << stat->total_recv << std::endl;
	std::cout << " time cost average : " << stat->time_cost_average << " max cost difference  :" << stat->time_cost_df_max << std::endl;
	std::cout << " -------------------------------- " << std::endl;
}

#endif



