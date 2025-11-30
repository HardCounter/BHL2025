import { Line } from "react-chartjs-2";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler,
} from "chart.js";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { ChartDataPoint } from "@/types/sensor-data";

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
);

interface SensorChartProps {
  title: string;
  data: ChartDataPoint[];
  unit?: string;
  color: string;
}

export const SensorChart = ({ title, data, unit = "", color }: SensorChartProps) => {
  const chartData = {
    labels: data.map((d) => {
      const time = d.timestamp.split(" ")[1];
      return time || d.timestamp;
    }),
    datasets: [
      {
        label: title,
        data: data.map((d) => d.value),
        borderColor: color,
        backgroundColor: `${color}20`,
        borderWidth: 2,
        fill: true,
        tension: 0.4,
        pointRadius: 2,
        pointHoverRadius: 4,
      },
    ],
  };

  const options = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: {
        display: false,
      },
      tooltip: {
        mode: "index" as const,
        intersect: false,
        backgroundColor: "hsl(var(--card))",
        titleColor: "hsl(var(--foreground))",
        bodyColor: "hsl(var(--foreground))",
        borderColor: "hsl(var(--border))",
        borderWidth: 1,
        padding: 12,
        displayColors: false,
        callbacks: {
          label: (context: any) => `${context.parsed.y.toFixed(2)}${unit}`,
        },
      },
    },
    scales: {
      x: {
        grid: {
          color: "hsl(var(--border))",
          drawTicks: false,
        },
        ticks: {
          color: "hsl(var(--muted-foreground))",
          maxRotation: 45,
          minRotation: 45,
          font: {
            size: 10,
          },
        },
        border: {
          display: false,
        },
      },
      y: {
        grid: {
          color: "hsl(var(--border))",
          drawTicks: false,
        },
        ticks: {
          color: "hsl(var(--muted-foreground))",
          font: {
            size: 11,
          },
          callback: (value: number) => `${value}${unit}`,
        },
        border: {
          display: false,
        },
      },
    },
    interaction: {
      mode: "nearest" as const,
      axis: "x" as const,
      intersect: false,
    },
  };

  return (
    <Card className="overflow-hidden">
      <CardHeader className="pb-4">
        <CardTitle className="text-lg flex items-center gap-2">
          <div
            className="w-3 h-3 rounded-full"
            style={{ backgroundColor: color }}
          />
          {title}
        </CardTitle>
      </CardHeader>
      <CardContent className="pt-0">
        <div className="h-[250px]">
          <Line data={chartData} options={options} />
        </div>
      </CardContent>
    </Card>
  );
};
