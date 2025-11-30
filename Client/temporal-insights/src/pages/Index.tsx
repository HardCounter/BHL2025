import { SensorChart } from "@/components/SensorChart";
import { ChartDataPoint, SensorData } from "@/types/sensor-data";
import { Activity, Loader2 } from "lucide-react";
import { Card, CardContent } from "@/components/ui/card";
import { useChartData } from "@/hooks/useChartData";
import { useEffect, useState } from "react";

const Chart = ({
  title,
  field,
  unit,
  color,
  setTotalMeasurements,
}: {
  title: string;
  field: keyof Omit<SensorData, "id" | "timestamp">;
  unit: string;
  color: string;
  setTotalMeasurements: (count: number) => void;
}) => {
  const { data, isLoading, error } = useChartData(field, 200);

  useEffect(() => {
    if (data) {
      setTotalMeasurements(data.length);
    }
  }, [data, setTotalMeasurements]);

  if (isLoading) {
    return (
      <div className="flex h-64 items-center justify-center">
        <Loader2 className="w-8 h-8 animate-spin text-primary" />
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex h-64 items-center justify-center p-4">
        <Card className="w-full max-w-md border-destructive">
          <CardContent className="pt-6">
            <div className="text-center space-y-2">
              <h2 className="text-xl font-semibold text-destructive">
                Błąd
              </h2>
              <p className="text-muted-foreground text-sm">
                Nie udało się pobrać danych dla {title}.
              </p>
            </div>
          </CardContent>
        </Card>
      </div>
    );
  }

  return (
    <SensorChart title={title} data={data || []} unit={unit} color={color} />
  );
};

const Index = () => {
  const [totalMeasurements, setTotalMeasurements] = useState(0);

  return (
    <div className="min-h-screen bg-background">
      <header className="border-b bg-card/50 backdrop-blur-sm sticky top-0 z-10">
        <div className="container mx-auto px-4 py-4">
          <div className="flex items-center gap-3">
            <Activity className="w-8 h-8 text-primary" />
            <div>
              <h1 className="text-2xl font-bold">
                Dashboard Monitoringu Sensorów
              </h1>
              <p className="text-sm text-muted-foreground">
                Dane w czasie rzeczywistym z {totalMeasurements} pomiarów
              </p>
            </div>
          </div>
        </div>
      </header>

      <main className="container mx-auto px-4 py-8">
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          <Chart
            title="Temperatura Powietrza"
            field="temp_air"
            unit="°C"
            color="hsl(var(--chart-1))"
            setTotalMeasurements={setTotalMeasurements}
          />
          <Chart
            title="Temperatura Ściany"
            field="temp_wall"
            unit="°C"
            color="hsl(var(--chart-2))"
            setTotalMeasurements={setTotalMeasurements}
          />
          <Chart
            title="Wilgotność Powietrza"
            field="wilg_pow"
            unit="%"
            color="hsl(var(--chart-3))"
            setTotalMeasurements={setTotalMeasurements}
          />
          <Chart
            title="Mech"
            field="mech"
            unit=""
            color="hsl(var(--chart-4))"
            setTotalMeasurements={setTotalMeasurements}
          />
          <Chart
            title="Woda"
            field="woda"
            unit=""
            color="hsl(var(--chart-5))"
            setTotalMeasurements={setTotalMeasurements}
          />
          <Chart
            title="Dym"
            field="dym"
            unit=""
            color="hsl(var(--chart-6))"
            setTotalMeasurements={setTotalMeasurements}
          />
          <Chart
            title="Ogień"
            field="ogien"
            unit=""
            color="hsl(var(--chart-7))"
            setTotalMeasurements={setTotalMeasurements}
          />
        </div>

        <footer className="mt-12 text-center text-sm text-muted-foreground">
          <p>Dane odświeżane automatycznie co 5 sekund</p>
        </footer>
      </main>
    </div>
  );
};

export default Index;
