#include <vector>
#include <iostream>

int neighbors[20][3] =
  {
   { 1,  2,  3}, //  0
   { 5,  0,  4}, //  1
   { 7,  0,  6}, //  2
   { 9,  0,  8}, //  3
   { 1,  9, 12}, //  4
   {11,  6,  1}, //  5
   { 2,  5, 10}, //  6
   {15,  8,  2}, //  7
   { 3,  7, 14}, //  8
   {13,  4,  3}, //  9
   { 6, 16, 15}, // 10
   {16,  5, 12}, // 11
   { 4, 17, 11}, // 12
   {17,  9, 14}, // 13
   { 8, 18, 13}, // 14
   {18,  7, 10}, // 15
   {10, 11, 19}, // 16
   {12, 13, 19}, // 17
   {14, 15, 19}, // 18
   {18, 16, 17}  // 19
  };


template < typename T >
std::ostream& operator<<(std::ostream& out, const std::vector < T > &a) {
  out << "[";
  for (int i=0; i<a.size(); ++i) {
    if (i > 0) out << ",";
    out << a[i];
  }
  out << "]";
  return out;
}

std::vector< std::vector < int > > round0;
std::vector< std::vector < int > > round1;

int abs(int x) { return x >= 0 ? x : -x; }

int best = 1000000;

int fit(const std::vector<int> &labels) {
  int maxans = 0, sumans=0;
  for (int i=0; i<10; ++i) {
    int sum = labels[i] + labels[19-i];
    int err = abs(sum-9);
    sumans += err;
    if (err > maxans) maxans=err;    
  }
  for (int i=0; i<20; ++i) {
    int sum = labels[i] + labels[neighbors[i][0]] + labels[neighbors[i][1]] + labels[neighbors[i][2]];
    int err = abs(sum-18);
    sumans += err;
    if (err > maxans) maxans=err;
  }
  int ans = 1000*maxans + sumans;
  if (ans < best) best=ans;
  return ans;
}

int fit(const std::vector<int> &labels) {
  int maxans = 0, sumans=0;
  for (int i=0; i<10; ++i) {
    int sum = labels[i] + labels[19-i];
    int err = abs(sum-9);
    sumans += err;
    if (err > maxans) maxans=err;    
  }
  for (int i=0; i<20; ++i) {
    int sum = labels[i] + labels[neighbors[i][0]] + labels[neighbors[i][1]] + labels[neighbors[i][2]];
    int err = abs(sum-18);
    sumans += err;
    if (err > maxans) maxans=err;
  }
  int ans = 1000*maxans + sumans;
  if (ans < best) best=ans;
  return ans;
}


void trials0(int p[9], int d)
{
  if (d == 0) {
    std::vector<int> labels(20,0);
    labels[0]=0;
    labels[19]=9;
    for (int i=0; i<9; ++i) {
      labels[i+1]=p[i]+1;
      labels[18-i]=9-(p[i]+1);
    }
    if (fit(labels) == 5048) {
      round0.push_back(labels);
    }
  } else {
    for (int i=0; i<=d; ++i) {
      int a=p[i],b=p[d];
      p[i]=b;
      p[d]=a;
      trials0(p,d-1);
      p[i]=a;
      p[d]=b;
    }
  }
}

void trials1(std::vector<int> &labels, int d, bool delta) {
  if (d == 0) {
    if (delta && labels[0]==0 && fit(labels) <= 4052) {
      if (labels[1] <= labels[2] && labels[1] <= labels[3] && labels[2] <= labels[3]) {
	round1.push_back(labels);
      }
    }
  } else {
    trials1(labels,d-1,delta);
    for (int i=0; i<d; ++i) {
      if (labels[i] == labels[d]-1) {
	int a=labels[i],b=labels[d];
        labels[i]=b;
        labels[d]=a;
        trials1(labels,d-1,true);
        labels[i]=a;
        labels[d]=b;
      }
    }
    for (int i=0; i<d; ++i) {
      if (labels[i] == labels[d]+1) {
	int a=labels[i],b=labels[d];
        labels[i]=b;
        labels[d]=a;
        trials1(labels,d-1,true);
        labels[i]=a;
        labels[d]=b;
      }
    }
  }
}

int main(int argc, const char *argv[]) {
  int p[9];
  for (int i=0; i<9; ++i) p[i]=i;
  best = 1000000;
  trials0(p,8);
  for (auto solution : round0) {
    std::cout << fit(solution) << " " << solution <<std::endl;
  }
  std::cout << round0.size() << " round0 solutions, best=" << best << std::endl;
  best = 1000000;
  for (auto labels : round0) {
    trials1(labels,19,false);
  }
  for (auto solution : round1) {
    std::cout << fit(solution) << " " << solution <<std::endl;
  }
  std::cout << round1.size() << " round1 solutions, best=" << best << std::endl;  
  return 0;
}
