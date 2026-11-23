import assert from 'node:assert/strict';
import { existsSync, mkdirSync, mkdtempSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { describe, it } from 'node:test';
import {
  loadOrInitCyclicState,
  restoreBucketFromB8p,
  runCyclicStep,
  type CyclicBucketSpec,
} from './cyclic-builder.mts';

describe('sectium-maintenance cyclic-builder (micro-cycle resumable .b8p shard build)', () => {
  it('executes 1 bucket per micro-cycle, packs SHA-256 .b8p archives, survives simulated failure, and restores cleanly', () => {
    const tempRoot = mkdtempSync(join(tmpdir(), 'b8g-cyclic-'));
    try {
      const outDir = join(tempRoot, 'out');
      const casDir = join(tempRoot, 'cas');
      const statePath = join(tempRoot, 'cyclic-state.json');

      const miniBuckets: CyclicBucketSpec[] = [
        { id: '01-zlib', ninjaTarget: 'third_party/zlib:zlib', outputGlobDir: 'obj/zlib', order: 1 },
        { id: '02-base', ninjaTarget: 'base:base', outputGlobDir: 'obj/base', order: 2 },
        { id: '03-v8',   ninjaTarget: 'v8:v8', outputGlobDir: 'obj/v8', order: 3 },
      ];

      let failOnceOnBase = true;
      const fakeRunner = (bucket: CyclicBucketSpec, dir: string): { ok: boolean; error?: string } => {
        if (bucket.id === '02-base' && failOnceOnBase) {
          failOnceOnBase = false;
          return { ok: false, error: 'Simulated transient OOM crash' };
        }
        const targetDir = join(dir, bucket.outputGlobDir);
        mkdirSync(targetDir, { recursive: true });
        writeFileSync(join(targetDir, `${bucket.id}.obj`), Buffer.from(`OBJ-BYTES-${bucket.id}`));
        return { ok: true };
      };

      // Cycle 1: builds 01-zlib -> DONE + .b8p
      const r1 = runCyclicStep({ statePath, outDir, casDir, buckets: miniBuckets, maxBucketsPerCycle: 1, runner: fakeRunner });
      assert.deepEqual(r1.completedThisCycle, ['01-zlib']);
      assert.equal(r1.totalDone, 1);
      assert.equal(r1.allDone, false);

      // Cycle 2: 02-base crashes transiently -> 01-zlib stays DONE!
      const r2 = runCyclicStep({ statePath, outDir, casDir, buckets: miniBuckets, maxBucketsPerCycle: 1, runner: fakeRunner });
      assert.deepEqual(r2.completedThisCycle, []);
      assert.equal(r2.totalDone, 1);

      // Cycle 3: resumes right at 02-base -> succeeds, then Cycle 4 finishes 03-v8!
      const r3 = runCyclicStep({ statePath, outDir, casDir, buckets: miniBuckets, maxBucketsPerCycle: 2, runner: fakeRunner });
      assert.deepEqual(r3.completedThisCycle, ['02-base', '03-v8']);
      assert.equal(r3.totalDone, 3);
      assert.equal(r3.allDone, true);

      // Verify .b8p restore into a brand-new clean out directory
      const state = loadOrInitCyclicState(statePath, outDir, casDir, miniBuckets);
      const freshOut = join(tempRoot, 'fresh-out');
      for (const b of state.buckets) {
        assert.equal(b.status, 'DONE');
        assert.ok(b.b8pPath && existsSync(b.b8pPath));
        const restored = restoreBucketFromB8p(b.b8pPath!, b.sha256!, freshOut);
        assert.equal(restored, 1);
      }
      assert.equal(
        readFileSync(join(freshOut, 'obj/v8/03-v8.obj'), 'utf8'),
        'OBJ-BYTES-03-v8',
      );
    } finally {
      rmSync(tempRoot, { recursive: true, force: true });
    }
  });
});
