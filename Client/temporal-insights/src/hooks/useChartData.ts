import { useQuery } from "@tanstack/react-query";
import { ChartDataPoint } from "@/types/sensor-data";

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL;

const fetchChartData = async (
  field: string,
  limit: number
): Promise<ChartDataPoint[]> => {
  const response = await fetch(
    `${API_BASE_URL}/readings/${field}?limit=${limit}`
  );
  if (!response.ok) {
    throw new Error(`Failed to fetch data for ${field}`);
  }
  const data = await response.json();
  return data;
};

export const useChartData = (field: string, limit: number = 100) => {
  return useQuery({
    queryKey: ["chartData", field, limit],
    queryFn: () => fetchChartData(field, limit),
    refetchInterval: 5000,
  });
};