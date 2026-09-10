import { describe, expect, it } from "vitest";
import type { TempoSegment } from "../../src/model/tempoSegment";
import { convertTickToAbsoluteSeconds } from "../../src/timing/convertTickToAbsoluteSeconds";

const ppq: number = 480;
const scoreEndTick: number = Number.MAX_SAFE_INTEGER;
const tempoSegments: TempoSegment[] = [
  {
    startTick: 0,
    endTick: 1,
    bpm: 100,
    startSeconds: 0
  },
  {
    startTick: 1,
    endTick: 100,
    bpm: 120,
    startSeconds: 0.00125
  },
  {
    startTick: 100,
    endTick: 480,
    bpm: 110,
    startSeconds: 0.104375
  },
  {
    startTick: 480,
    endTick: 5000,
    bpm: 200,
    startSeconds: 0.5361931818181818
  },
  {
    startTick: 5000,
    endTick: 9999,
    bpm: 50,
    startSeconds: 3.361193181818182
  },
  {
    startTick: 9999,
    endTick: 9_007_199_254_740_989,
    bpm: 20,
    startSeconds: 15.858693181818182,
  },
  {
    startTick: Number.MAX_SAFE_INTEGER - 2,
    endTick: null,
    bpm: 300,
    startSeconds: 56_294_995_342_084.555
  }
];

describe("convertTickToAbsoluteSeconds", () => {
  it.each([
    [ 0,          0           ],
    [ 1,          0.00125     ],
    [ 31,         0.0325      ],
    [ 100,        0.104375    ],
    [ 300,        0.33164773  ],
    [ 480,        0.53619318  ],
    [ 500,        0.54869318  ],
    [ 5000,       3.36119318  ],
    [ 7000,       8.36119318  ],
    [ 9999,      15.85869318  ],
    [ 999_999, 6203.35869318  ],
    [ Number.MAX_SAFE_INTEGER - 2, 56_294_995_342_084.555 ]
  ] as const)(
    "converts %i to %i absolute second(s)", (
      tick,
      expected
    ) => {
      expect(convertTickToAbsoluteSeconds(
        ppq,
        tick,
        tempoSegments,
        scoreEndTick
      )).toBeCloseTo(expected, 8); // Compare up to 8 decimal places
  });
  /**
	 * Phase 1 validation and buildTempoSegments guarantee
	 * that every playback tick is covered by a tempo segment.
	 * Reaching this point indicates an internal invariant violation. */
  it.each([
    -1,
    -42,
    -0.5,
    Number.NaN,
    Number.POSITIVE_INFINITY,
    Number.MAX_SAFE_INTEGER + 1,
  ])("throws for an uncovered tick: %i", (invalidTick) => {
    expect(() => convertTickToAbsoluteSeconds(
      ppq,
      invalidTick,
      tempoSegments,
      scoreEndTick
    )).toThrow(
      `Invariant violation: no tempo segment covers tick ${invalidTick}.`
    );
  })
});
