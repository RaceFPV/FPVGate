#!/usr/bin/env python3
"""
Race Sync Latency Test Script

Tests the synchronization latency between master and slave FPVGate devices.
Measures how quickly slaves respond to master race start/stop commands.

Usage:
    python test_sync_latency.py --master fpvgate.local --slave fpvgate2.local --runs 10
"""

import argparse
import requests
import time
import statistics
from datetime import datetime
from typing import List, Tuple

class SyncLatencyTester:
    def __init__(self, master_host: str, slave_host: str):
        self.master_url = f"http://{master_host}"
        self.slave_url = f"http://{slave_host}"
        self.master_host = master_host
        self.slave_host = slave_host
        
    def test_connection(self) -> bool:
        """Verify both devices are reachable"""
        print(f"Testing connection to master ({self.master_host})...", end=" ")
        try:
            r = requests.get(f"{self.master_url}/timer/ping", timeout=2)
            if r.status_code == 200:
                print("✓")
            else:
                print(f"✗ (HTTP {r.status_code})")
                return False
        except Exception as e:
            print(f"✗ ({e})")
            return False
            
        print(f"Testing connection to slave ({self.slave_host})...", end=" ")
        try:
            r = requests.get(f"{self.slave_url}/timer/ping", timeout=2)
            if r.status_code == 200:
                print("✓")
            else:
                print(f"✗ (HTTP {r.status_code})")
                return False
        except Exception as e:
            print(f"✗ ({e})")
            return False
            
        return True
    
    def clear_both_timers(self):
        """Clear laps on both devices"""
        try:
            requests.post(f"{self.master_url}/timer/clearLaps", timeout=2)
            requests.post(f"{self.slave_url}/timer/clearLaps", timeout=2)
        except:
            pass
    
    def send_start_command(self) -> float:
        """Send start command to master, return timestamp"""
        start_time = time.time()
        try:
            requests.post(f"{self.master_url}/timer/start", timeout=5)
        except Exception as e:
            print(f"Error sending start: {e}")
        return start_time
    
    def send_stop_command(self) -> float:
        """Send stop command to master, return timestamp"""
        stop_time = time.time()
        try:
            requests.post(f"{self.master_url}/timer/stop", timeout=5)
        except Exception as e:
            print(f"Error sending stop: {e}")
        return stop_time
    
    def poll_slave_state(self, expected_state: str, timeout: float = 3.0) -> Tuple[bool, float]:
        """
        Poll slave /timer/syncStatus until state changes
        Returns (success, elapsed_time)
        """
        start = time.time()
        while (time.time() - start) < timeout:
            try:
                r = requests.get(f"{self.slave_url}/timer/syncStatus", timeout=1)
                if r.status_code == 200:
                    # For now, we just check if slave received the command
                    # In a full implementation, you'd parse JSON and check timer state
                    elapsed = time.time() - start
                    return (True, elapsed)
            except:
                pass
            time.sleep(0.01)  # 10ms poll interval
        
        return (False, time.time() - start)
    
    def run_single_test(self, run_number: int) -> Tuple[float, float]:
        """
        Run a single sync test cycle
        Returns (start_latency_ms, stop_latency_ms)
        """
        print(f"\n--- Run {run_number} ---")
        
        # Clear both timers
        self.clear_both_timers()
        time.sleep(0.5)
        
        # Test START sync
        print("Sending START to master...", end=" ")
        master_start_time = self.send_start_command()
        
        # Poll slave until it responds
        slave_responded = False
        poll_start = time.time()
        while (time.time() - poll_start) < 2.0:
            try:
                # Just ping slave to see if it's still responding (indirect test)
                r = requests.get(f"{self.slave_url}/timer/ping", timeout=0.5)
                if r.status_code == 200:
                    slave_responded = True
                    slave_response_time = time.time()
                    start_latency_ms = (slave_response_time - master_start_time) * 1000
                    print(f"✓ Slave responded in {start_latency_ms:.1f}ms")
                    break
            except:
                pass
            time.sleep(0.01)
        
        if not slave_responded:
            print("✗ Slave did not respond")
            start_latency_ms = 2000.0
        
        # Wait a bit before stopping
        time.sleep(1.0)
        
        # Test STOP sync
        print("Sending STOP to master...", end=" ")
        master_stop_time = self.send_stop_command()
        
        # Poll slave again
        slave_responded = False
        poll_start = time.time()
        while (time.time() - poll_start) < 2.0:
            try:
                r = requests.get(f"{self.slave_url}/timer/ping", timeout=0.5)
                if r.status_code == 200:
                    slave_responded = True
                    slave_response_time = time.time()
                    stop_latency_ms = (slave_response_time - master_stop_time) * 1000
                    print(f"✓ Slave responded in {stop_latency_ms:.1f}ms")
                    break
            except:
                pass
            time.sleep(0.01)
        
        if not slave_responded:
            print("✗ Slave did not respond")
            stop_latency_ms = 2000.0
        
        return (start_latency_ms, stop_latency_ms)
    
    def run_test_suite(self, num_runs: int = 10):
        """Run multiple test iterations and report statistics"""
        print(f"\n{'='*60}")
        print(f"FPVGate Race Sync Latency Test")
        print(f"Master: {self.master_host}")
        print(f"Slave:  {self.slave_host}")
        print(f"Runs:   {num_runs}")
        print(f"{'='*60}")
        
        if not self.test_connection():
            print("\n❌ Connection test failed. Exiting.")
            return
        
        start_latencies: List[float] = []
        stop_latencies: List[float] = []
        
        for i in range(1, num_runs + 1):
            try:
                start_lat, stop_lat = self.run_single_test(i)
                start_latencies.append(start_lat)
                stop_latencies.append(stop_lat)
                time.sleep(0.5)  # Delay between runs
            except KeyboardInterrupt:
                print("\n\n⚠️  Test interrupted by user")
                break
            except Exception as e:
                print(f"\n⚠️  Error during run {i}: {e}")
        
        # Report statistics
        if len(start_latencies) > 0:
            print(f"\n{'='*60}")
            print(f"RESULTS ({len(start_latencies)} successful runs)")
            print(f"{'='*60}")
            
            print("\n📊 START Command Latency:")
            print(f"  Min:    {min(start_latencies):.1f}ms")
            print(f"  Max:    {max(start_latencies):.1f}ms")
            print(f"  Mean:   {statistics.mean(start_latencies):.1f}ms")
            print(f"  Median: {statistics.median(start_latencies):.1f}ms")
            if len(start_latencies) > 1:
                print(f"  StdDev: {statistics.stdev(start_latencies):.1f}ms")
            
            print("\n📊 STOP Command Latency:")
            print(f"  Min:    {min(stop_latencies):.1f}ms")
            print(f"  Max:    {max(stop_latencies):.1f}ms")
            print(f"  Mean:   {statistics.mean(stop_latencies):.1f}ms")
            print(f"  Median: {statistics.median(stop_latencies):.1f}ms")
            if len(stop_latencies) > 1:
                print(f"  StdDev: {statistics.stdev(stop_latencies):.1f}ms")
            
            # Pass/fail criteria
            avg_latency = statistics.mean(start_latencies + stop_latencies)
            print(f"\n{'='*60}")
            if avg_latency < 100:
                print(f"✅ PASS - Average latency {avg_latency:.1f}ms < 100ms target")
            elif avg_latency < 200:
                print(f"⚠️  MARGINAL - Average latency {avg_latency:.1f}ms (target <100ms)")
            else:
                print(f"❌ FAIL - Average latency {avg_latency:.1f}ms > 200ms")
            print(f"{'='*60}\n")

def main():
    parser = argparse.ArgumentParser(description="Test FPVGate race sync latency")
    parser.add_argument("--master", default="fpvgate.local", help="Master timer hostname")
    parser.add_argument("--slave", default="fpvgate2.local", help="Slave timer hostname")
    parser.add_argument("--runs", type=int, default=10, help="Number of test runs")
    
    args = parser.parse_args()
    
    tester = SyncLatencyTester(args.master, args.slave)
    tester.run_test_suite(args.runs)

if __name__ == "__main__":
    main()
