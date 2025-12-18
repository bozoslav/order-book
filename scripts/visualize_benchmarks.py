#!/usr/bin/env python3
import json
import sys
import argparse
from pathlib import Path
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
from typing import Dict, List, Any

matplotlib.use('Agg')

def parse_benchmark_json(json_file: Path) -> Dict[str, Any]:
    with open(json_file, 'r') as f:
        data = json.load(f)
    return data

def extract_latency_benchmarks(data: Dict[str, Any]) -> List[Dict[str, Any]]:
    latency_benchmarks = []
    for benchmark in data.get('benchmarks', []):
        name = benchmark.get('name', '')
        if 'Latency' in name or 'AddOrder' in name or 'CancelOrder' in name:
            if 'Throughput' not in name:
                latency_benchmarks.append(benchmark)
    return latency_benchmarks

def extract_throughput_benchmarks(data: Dict[str, Any]) -> List[Dict[str, Any]]:
    throughput_benchmarks = []
    for benchmark in data.get('benchmarks', []):
        name = benchmark.get('name', '')
        if 'Throughput' in name and 'items_per_second' in benchmark:
            throughput_benchmarks.append(benchmark)
    return throughput_benchmarks

def calculate_percentiles(times: List[float]) -> Dict[str, float]:
    if not times:
        return {'p50': 0, 'p99': 0, 'p99.9': 0}
    
    times_sorted = sorted(times)
    n = len(times_sorted)
    
    return {
        'p50': times_sorted[int(n * 0.50)],
        'p99': times_sorted[int(n * 0.99)] if n > 100 else times_sorted[-1],
        'p99.9': times_sorted[int(n * 0.999)] if n > 1000 else times_sorted[-1],
    }

def plot_latency_comparison(latency_benchmarks: List[Dict[str, Any]], output_dir: Path):
    if not latency_benchmarks:
        print("No latency benchmarks found")
        return
    
    benchmark_groups = {}
    for bench in latency_benchmarks:
        name = bench.get('name', '')
        base_name = name.split('/')[0]
        time_ns = bench.get('real_time', 0)
        
        if base_name not in benchmark_groups:
            benchmark_groups[base_name] = []
        benchmark_groups[base_name].append(time_ns)
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))
    
    names = []
    mean_times = []
    
    for name, times in sorted(benchmark_groups.items()):
        if times:
            names.append(name.replace('BM_', '').replace('_', ' '))
            mean_times.append(np.mean(times))
    
    if names:
        colors = plt.cm.viridis(np.linspace(0, 1, len(names)))
        bars = ax1.barh(names, mean_times, color=colors)
        ax1.set_xlabel('Latency (nanoseconds)', fontsize=12)
        ax1.set_title('Mean Latency by Operation', fontsize=14, fontweight='bold')
        ax1.grid(axis='x', alpha=0.3)
        
        for i, (bar, val) in enumerate(zip(bars, mean_times)):
            ax1.text(val, bar.get_y() + bar.get_height()/2, f' {val:.0f} ns', 
                    va='center', fontsize=9)
    
    percentile_data = {}
    for bench in latency_benchmarks:
        name = bench.get('name', '')
        if 'LatencyDistribution' in name:
            base_name = name.replace('BM_', '').replace('_LatencyDistribution', '')
            time_ns = bench.get('real_time', 0)
            
            if base_name not in percentile_data:
                percentile_data[base_name] = []
            percentile_data[base_name].append(time_ns)
    
    if percentile_data:
        x = np.arange(len(percentile_data))
        width = 0.25
        
        p50_values = []
        p99_values = []
        p999_values = []
        labels = []
        
        for name, times in sorted(percentile_data.items()):
            percentiles = calculate_percentiles(times)
            labels.append(name.replace('_', ' '))
            p50_values.append(percentiles['p50'])
            p99_values.append(percentiles['p99'])
            p999_values.append(percentiles['p99.9'])
        
        ax2.bar(x - width, p50_values, width, label='p50', color='green', alpha=0.8)
        ax2.bar(x, p99_values, width, label='p99', color='orange', alpha=0.8)
        ax2.bar(x + width, p999_values, width, label='p99.9', color='red', alpha=0.8)
        
        ax2.set_ylabel('Latency (nanoseconds)', fontsize=12)
        ax2.set_title('Latency Percentiles', fontsize=14, fontweight='bold')
        ax2.set_xticks(x)
        ax2.set_xticklabels(labels, rotation=45, ha='right')
        ax2.legend()
        ax2.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    output_file = output_dir / 'latency_comparison.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved latency comparison to {output_file}")
    plt.close()

def plot_throughput_comparison(throughput_benchmarks: List[Dict[str, Any]], output_dir: Path):
    if not throughput_benchmarks:
        print("No throughput benchmarks found")
        return
    
    benchmark_data = []
    for bench in throughput_benchmarks:
        name = bench.get('name', '')
        base_name = name.split('/')[0].replace('BM_', '').replace('_', ' ')
        range_param = name.split('/')[-1] if '/' in name else ''
        throughput = bench.get('items_per_second', 0)
        
        benchmark_data.append({
            'name': base_name,
            'range': range_param,
            'throughput': throughput
        })
    
    grouped_data = {}
    for item in benchmark_data:
        name = item['name']
        if name not in grouped_data:
            grouped_data[name] = []
        grouped_data[name].append(item)
    
    n_groups = len(grouped_data)
    if n_groups == 0:
        return
    
    fig, axes = plt.subplots(1, min(n_groups, 3), figsize=(6 * min(n_groups, 3), 6))
    if n_groups == 1:
        axes = [axes]
    
    for idx, (name, items) in enumerate(sorted(grouped_data.items())[:3]):
        ax = axes[idx] if n_groups > 1 else axes[0]
        
        items_sorted = sorted(items, key=lambda x: int(x['range']) if x['range'].isdigit() else 0)
        
        if len(items_sorted) > 1:
            ranges = [int(item['range']) if item['range'].isdigit() else 0 for item in items_sorted]
            throughputs = [item['throughput'] for item in items_sorted]
            
            ax.plot(ranges, throughputs, marker='o', linewidth=2, markersize=8)
            ax.set_xlabel('Number of Orders', fontsize=11)
            ax.set_ylabel('Orders per Second', fontsize=11)
            ax.set_title(name, fontsize=12, fontweight='bold')
            ax.grid(True, alpha=0.3)
            
            for x, y in zip(ranges, throughputs):
                ax.annotate(f'{y:.0f}', (x, y), textcoords="offset points", 
                           xytext=(0,10), ha='center', fontsize=9)
        else:
            throughput = items_sorted[0]['throughput']
            ax.bar([name], [throughput], color='steelblue', alpha=0.8)
            ax.set_ylabel('Orders per Second', fontsize=11)
            ax.set_title(name, fontsize=12, fontweight='bold')
            ax.tick_params(axis='x', rotation=45)
            ax.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    output_file = output_dir / 'throughput_comparison.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved throughput comparison to {output_file}")
    plt.close()

def plot_scaling_analysis(latency_benchmarks: List[Dict[str, Any]], output_dir: Path):
    scaling_benchmarks = []
    for bench in latency_benchmarks:
        name = bench.get('name', '')
        if '/' in name:
            scaling_benchmarks.append(bench)
    
    if not scaling_benchmarks:
        return
    
    grouped = {}
    for bench in scaling_benchmarks:
        name = bench.get('name', '')
        base_name = name.split('/')[0]
        range_param = int(name.split('/')[-1])
        time_ns = bench.get('real_time', 0)
        
        if base_name not in grouped:
            grouped[base_name] = []
        grouped[base_name].append((range_param, time_ns))
    
    if not grouped:
        return
    
    fig, ax = plt.subplots(figsize=(12, 8))
    
    for name, data in sorted(grouped.items()):
        data_sorted = sorted(data, key=lambda x: x[0])
        ranges = [x[0] for x in data_sorted]
        times = [x[1] for x in data_sorted]
        
        label = name.replace('BM_', '').replace('_', ' ')
        ax.plot(ranges, times, marker='o', linewidth=2, label=label, markersize=6)
    
    ax.set_xlabel('Number of Orders in Book', fontsize=12)
    ax.set_ylabel('Latency (nanoseconds)', fontsize=12)
    ax.set_title('Operation Latency vs. Book Size', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_xscale('log')
    ax.set_yscale('log')
    
    plt.tight_layout()
    output_file = output_dir / 'scaling_analysis.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved scaling analysis to {output_file}")
    plt.close()

def generate_summary_report(data: Dict[str, Any], output_dir: Path):
    output_file = output_dir / 'benchmark_summary.txt'
    
    with open(output_file, 'w') as f:
        f.write("=" * 80 + "\n")
        f.write("ORDER BOOK PERFORMANCE BENCHMARK SUMMARY\n")
        f.write("=" * 80 + "\n\n")
        
        context = data.get('context', {})
        f.write(f"Date: {context.get('date', 'N/A')}\n")
        f.write(f"Host: {context.get('host_name', 'N/A')}\n")
        f.write(f"CPU: {context.get('cpu_scaling_enabled', 'N/A')}\n")
        f.write(f"CPUs: {context.get('num_cpus', 'N/A')}\n")
        f.write(f"Caches: {context.get('caches', 'N/A')}\n\n")
        
        f.write("LATENCY BENCHMARKS (Hot Path Operations)\n")
        f.write("-" * 80 + "\n")
        
        latency_benchmarks = extract_latency_benchmarks(data)
        if latency_benchmarks:
            for bench in sorted(latency_benchmarks, key=lambda x: x.get('real_time', 0)):
                name = bench.get('name', '')
                time_ns = bench.get('real_time', 0)
                time_us = time_ns / 1000.0
                cpu_time = bench.get('cpu_time', 0)
                
                f.write(f"\n{name}:\n")
                f.write(f"  Real Time: {time_ns:.2f} ns ({time_us:.3f} µs)\n")
                f.write(f"  CPU Time:  {cpu_time:.2f} ns\n")
        
        f.write("\n\n")
        f.write("THROUGHPUT BENCHMARKS\n")
        f.write("-" * 80 + "\n")
        
        throughput_benchmarks = extract_throughput_benchmarks(data)
        if throughput_benchmarks:
            for bench in sorted(throughput_benchmarks, 
                              key=lambda x: x.get('items_per_second', 0), 
                              reverse=True):
                name = bench.get('name', '')
                throughput = bench.get('items_per_second', 0)
                
                f.write(f"\n{name}:\n")
                f.write(f"  Throughput: {throughput:,.0f} orders/second\n")
                f.write(f"  Throughput: {throughput/1_000_000:.2f} million orders/second\n")
        
        f.write("\n" + "=" * 80 + "\n")
    
    print(f"Saved summary report to {output_file}")

def main():
    parser = argparse.ArgumentParser(
        description='Visualize Google Benchmark results for Order Book'
    )
    parser.add_argument(
        'json_file',
        type=Path,
        help='Path to the benchmark JSON output file'
    )
    parser.add_argument(
        '-o', '--output-dir',
        type=Path,
        default=Path('benchmark_results'),
        help='Output directory for generated graphs (default: benchmark_results)'
    )
    
    args = parser.parse_args()
    
    if not args.json_file.exists():
        print(f"Error: JSON file not found: {args.json_file}")
        sys.exit(1)
    
    args.output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"Parsing benchmark results from {args.json_file}...")
    data = parse_benchmark_json(args.json_file)
    
    latency_benchmarks = extract_latency_benchmarks(data)
    throughput_benchmarks = extract_throughput_benchmarks(data)
    
    print(f"Found {len(latency_benchmarks)} latency benchmarks")
    print(f"Found {len(throughput_benchmarks)} throughput benchmarks")
    
    print("\nGenerating visualizations...")
    plot_latency_comparison(latency_benchmarks, args.output_dir)
    plot_throughput_comparison(throughput_benchmarks, args.output_dir)
    plot_scaling_analysis(latency_benchmarks, args.output_dir)
    generate_summary_report(data, args.output_dir)
    
    print(f"\nAll visualizations saved to {args.output_dir}/")

if __name__ == '__main__':
    main()
