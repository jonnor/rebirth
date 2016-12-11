int int_cmp(const void *a, const void *b) { 
  const int *ia = (const int *)a;
  const int *ib = (const int *)b;
  return *ia  - *ib;
}

template <int Length>
class Averager {
public:
  // collect values
  void push(int value) {
    buffer[index++] = value;
    if (index >= size) {
      index = 0;
    }
  }
  // calculate median of the values
  // NOTE: if not completely filled yet, will give arbitrary result
  int getMedian() {
    int samples[size];
    qsort(samples, size, sizeof(int), int_cmp);
    return samples[size/2]; 
  }

private:
  int buffer[Length];
  const int size = Length;
  int index = 0;
};
