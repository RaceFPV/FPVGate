/**
 * FPVGate Race Sync Latency Test - Browser Console Version
 * 
 * Run this in the browser console on the MASTER device's web UI.
 * It will measure how long it takes for the slave to receive start/stop commands.
 * 
 * Usage:
 *   1. Open master UI in browser (e.g., http://fpvgate1.local or http://192.168.0.158)
 *   2. Open browser console (F12 -> Console)
 *   3. Paste this entire script and press Enter
 *   4. Run: testSync('fpvgate2.local', 5)  // or use IP: testSync('192.168.0.177', 5)
 */

async function testSync(slaveHost, numRuns = 5) {
    const slaveUrl = `http://${slaveHost}`;
    const results = { start: [], stop: [] };
    
    console.log('='.repeat(60));
    console.log('FPVGate Race Sync Latency Test (Browser)');
    console.log(`Slave: ${slaveHost}`);
    console.log(`Runs: ${numRuns}`);
    console.log('='.repeat(60));
    
    // Test slave connectivity
    console.log(`\nTesting connection to slave...`);
    try {
        const pingStart = performance.now();
        const resp = await fetch(`${slaveUrl}/timer/ping`, { method: 'GET' });
        const pingTime = performance.now() - pingStart;
        if (resp.ok) {
            console.log(`✓ Slave reachable (ping: ${pingTime.toFixed(1)}ms)`);
        } else {
            console.error(`✗ Slave returned HTTP ${resp.status}`);
            return;
        }
    } catch (e) {
        console.error(`✗ Cannot reach slave: ${e.message}`);
        return;
    }
    
    for (let run = 1; run <= numRuns; run++) {
        console.log(`\n--- Run ${run}/${numRuns} ---`);
        
        // Clear laps first
        await fetch('/timer/clearLaps', { method: 'POST' });
        await new Promise(r => setTimeout(r, 300));
        
        // TEST START
        const startTime = performance.now();
        
        // Send start to local (master) device - this triggers device-to-device sync
        await fetch('/timer/start', { method: 'POST' });
        
        // Immediately check if slave received it by pinging
        let slaveStartReceived = false;
        let startLatency = 0;
        
        for (let i = 0; i < 50; i++) { // Poll for up to 500ms
            try {
                const checkStart = performance.now();
                const resp = await fetch(`${slaveUrl}/timer/ping`, { 
                    method: 'GET',
                    cache: 'no-store'
                });
                if (resp.ok) {
                    startLatency = performance.now() - startTime;
                    slaveStartReceived = true;
                    break;
                }
            } catch (e) {
                // Slave might be busy processing
            }
            await new Promise(r => setTimeout(r, 10));
        }
        
        if (slaveStartReceived) {
            console.log(`START: ${startLatency.toFixed(1)}ms`);
            results.start.push(startLatency);
        } else {
            console.log(`START: TIMEOUT`);
            results.start.push(2000);
        }
        
        // Wait before stopping
        await new Promise(r => setTimeout(r, 500));
        
        // TEST STOP
        const stopTime = performance.now();
        
        await fetch('/timer/stop', { method: 'POST' });
        
        let slaveStopReceived = false;
        let stopLatency = 0;
        
        for (let i = 0; i < 50; i++) {
            try {
                const resp = await fetch(`${slaveUrl}/timer/ping`, { 
                    method: 'GET',
                    cache: 'no-store'
                });
                if (resp.ok) {
                    stopLatency = performance.now() - stopTime;
                    slaveStopReceived = true;
                    break;
                }
            } catch (e) {}
            await new Promise(r => setTimeout(r, 10));
        }
        
        if (slaveStopReceived) {
            console.log(`STOP:  ${stopLatency.toFixed(1)}ms`);
            results.stop.push(stopLatency);
        } else {
            console.log(`STOP:  TIMEOUT`);
            results.stop.push(2000);
        }
        
        // Delay between runs
        await new Promise(r => setTimeout(r, 500));
    }
    
    // Calculate statistics
    const stats = (arr) => {
        const sorted = [...arr].sort((a, b) => a - b);
        const sum = arr.reduce((a, b) => a + b, 0);
        const mean = sum / arr.length;
        const median = sorted[Math.floor(sorted.length / 2)];
        const min = Math.min(...arr);
        const max = Math.max(...arr);
        const variance = arr.reduce((acc, val) => acc + Math.pow(val - mean, 2), 0) / arr.length;
        const stddev = Math.sqrt(variance);
        return { min, max, mean, median, stddev };
    };
    
    const startStats = stats(results.start);
    const stopStats = stats(results.stop);
    const allLatencies = [...results.start, ...results.stop];
    const avgLatency = allLatencies.reduce((a, b) => a + b, 0) / allLatencies.length;
    
    console.log('\n' + '='.repeat(60));
    console.log(`RESULTS (${numRuns} runs)`);
    console.log('='.repeat(60));
    
    console.log('\n START Command:');
    console.log(`  Min:    ${startStats.min.toFixed(1)}ms`);
    console.log(`  Max:    ${startStats.max.toFixed(1)}ms`);
    console.log(`  Mean:   ${startStats.mean.toFixed(1)}ms`);
    console.log(`  Median: ${startStats.median.toFixed(1)}ms`);
    console.log(`  StdDev: ${startStats.stddev.toFixed(1)}ms`);
    
    console.log('\n STOP Command:');
    console.log(`  Min:    ${stopStats.min.toFixed(1)}ms`);
    console.log(`  Max:    ${stopStats.max.toFixed(1)}ms`);
    console.log(`  Mean:   ${stopStats.mean.toFixed(1)}ms`);
    console.log(`  Median: ${stopStats.median.toFixed(1)}ms`);
    console.log(`  StdDev: ${stopStats.stddev.toFixed(1)}ms`);
    
    console.log('\n' + '='.repeat(60));
    if (avgLatency < 100) {
        console.log(` PASS - Average: ${avgLatency.toFixed(1)}ms (target <100ms)`);
    } else if (avgLatency < 200) {
        console.log(` MARGINAL - Average: ${avgLatency.toFixed(1)}ms (target <100ms)`);
    } else {
        console.log(` HIGH LATENCY - Average: ${avgLatency.toFixed(1)}ms`);
    }
    console.log('='.repeat(60));
    
    return { start: startStats, stop: stopStats, avgLatency };
}

// Quick test function
async function quickTest(slaveHost) {
    return testSync(slaveHost, 3);
}

console.log('✓ Sync test loaded. Run: testSync("fpvgate2.local", 5) or testSync("192.168.0.177", 5)');
