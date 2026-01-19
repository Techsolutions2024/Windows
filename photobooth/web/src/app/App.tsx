import { useEffect, useMemo, useState } from 'react';
import './setup';
import { AdminPage } from '../pages/AdminPage';
import { KioskPage } from '../pages/KioskPage';
import { AppShell } from '../components/AppShell';

type Route = 'kiosk' | 'admin';

function useHashRoute(): [Route, (r: Route) => void] {
  const getRoute = (): Route => {
    const h = window.location.hash.replace('#', '');
    if (h === 'admin') return 'admin';
    return 'kiosk';
  };

  const [route, setRoute] = useState<Route>(getRoute);

  useEffect(() => {
    const onHashChange = () => setRoute(getRoute());
    window.addEventListener('hashchange', onHashChange);
    return () => window.removeEventListener('hashchange', onHashChange);
  }, []);

  const navigate = (r: Route) => {
    window.location.hash = r === 'kiosk' ? '' : `#${r}`;
  };

  return [route, navigate];
}

export function App() {
  const [route, navigate] = useHashRoute();

  const isAdmin = useMemo(() => route === 'admin', [route]);

  return (
    <AppShell
      mode={isAdmin ? 'admin' : 'kiosk'}
      onGoKiosk={() => navigate('kiosk')}
      onGoAdmin={() => navigate('admin')}
    >
      {route === 'admin' ? <AdminPage /> : <KioskPage />}
    </AppShell>
  );
}
