
//#include "../rebirth/animation.hpp"
#include "../rebirth/simulate.hpp"

#include <assert.h>

bool
tests(void) {
    RgbColor midGray = { 100, 100, 100 };
    RgbColor pureRed = { 255, 0, 0 }; 
    
    // mix()
    RgbColor fullA = mix(midGray, pureRed, 0, 1000);
    assert(fullA == midGray);

    RgbColor fullB = mix(midGray, pureRed, 1000, 1000);
    assert(fullB == pureRed);

    RgbColor mid = mix(midGray, pureRed, 500, 1000);
    RgbColor midExpected{ 177, 50, 50 };
    //fprintf(stderr, "%s\n", mid.to_string().c_str());
    assert(mid == midExpected);

    RgbColor quarterA = mix(midGray, pureRed, 250, 1000);
    //fprintf(stderr, "%s\n", quarterA.to_string().c_str());
    assert(quarterA == (RgbColor{ 138, 75, 75 }));

    // scaleBrightness()
    RgbColor noBright = scaleBrightness(midGray, 0, 255);
    //fprintf(stderr, "%s\n", quarterA.to_string().c_str());
    assert(noBright == (RgbColor{ 0, 0, 0 }));

    RgbColor halfBright = scaleBrightness(midGray, 128, 255);
    //fprintf(stderr, "%s\n", halfBright.to_string().c_str());
    assert(halfBright == (RgbColor{ 50, 50, 50 }));

    RgbColor fullBright = scaleBrightness(midGray, 255, 255);
    //fprintf(stderr, "%s\n", fullBright.to_string().c_str());
    assert(fullBright == midGray);

    // triangleWave()
    const int waveMin = 10;
    const int waveMax = 255;
    const int oneSecondMs = 1000; 
    auto triangleZero = triangleWave(0, oneSecondMs, waveMin, waveMax);
    assert(triangleZero == waveMin);

    auto triangleOnePeriod = triangleWave(oneSecondMs,
                                oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleOnePeriod);
    assert(triangleOnePeriod == waveMin);

    auto triangleHalfUppeak = triangleWave((oneSecondMs/2)-10,
                                oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleHalfUppeak);
    assert(triangleHalfUppeak == waveMax-5);

    auto triangleHalf = triangleWave((oneSecondMs/2),
                            oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleHalf);
    assert(triangleHalf == waveMax);

    auto triangleHalfDownpeak = triangleWave((oneSecondMs/2)+10,
                                oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleHalfDownpeak);
    assert(triangleHalfDownpeak == waveMax-4);

    return true;
}

int
main(int argc, char *argv[]) {
    tests();

    auto history = simulateAnimation();
    auto trace = createFlowtrace(history);
    // TODO: write to file
    //printf("%s", trace.dump().c_str());
}
