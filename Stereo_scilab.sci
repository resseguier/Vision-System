// Modelisation d un systeme stereo
// Chaque camera est reperee par une lettre G et D
// Un modele de stenoppe classique est utilise
// Une mire unique en 3D permettra d avoir le jeu de points de calibration
//
// Made in TRIBOULET 2013
// Modification/improvment by RESSEGUIER 2014
clear;clc;


//Début - Récupère les coordonnées des points dans les .txt
    // donnees 3D de la mire de calibration en m
    xyz = fscanfMat('3D_init.txt');

    // Donnees  visualisation camera gauche en pixels
    uv_g = fscanfMat('2D_gauche.txt');

    // Donnees  visualisation camera droite en pixels
    uv_d = fscanfMat('2D_droite.txt');
//Fin

n=size(xyz,1);

x = xyz(:,1);
y = xyz(:,2);
z = xyz(:,3);

u_g = uv_g(:,1);
v_g = uv_g(:,2);
u_d = uv_d(:,1);
v_d = uv_d(:,2);

//Début - Camera Gauche
//Matrice A
A_cam_g = [x,y,z,ones(n,1),zeros(n,4),-(u_g) .*(x),-(u_g) .*(y),-(u_g) .*(z);
     zeros(n,4),x,y,z,ones(n,1),-(v_g) .*(x),-(v_g) .*(y),-(v_g) .*(z)];

//Matrice B
B_cam_g = [u_g;v_g];

// Parametres du modele par calcul de la pseudo inverse
C_g = (pinv(A_cam_g))*(B_cam_g);

// Ecriture matricielle du modele global -> matrice H !!
Glob_g = [(C_g(1:4))';
     (C_g(5:8))';
     (C_g(9:11))',1];

// Reconstruction des pixels a partir des points 3D et du modele estime
B_cam_g_rec = Glob_g*[(x');(y');(z');ones(1,n)];
u_g_rec = B_cam_g_rec(1,:) ./B_cam_g_rec(3,:);
v_g_rec = B_cam_g_rec(2,:) ./B_cam_g_rec(3,:);

// Erreur de reconstruction
Err_cam_g = [u_g'-u_g_rec;v_g'-v_g_rec];

// Max Min Moyenne Ecart type
Max_Err_cam_g = max(Err_cam_g,'c');
Min_Err_cam_g = min(Err_cam_g,'c');
Moy_Err_cam_g = mean(Err_cam_g,'c');
Std_Err_cam_g = stdev(Err_cam_g,'c');

//Camera Droite
//Matrice A
A_cam_d = [x,y,z,ones(n,1),zeros(n,4),-(u_d) .*(x),-(u_d) .*(y),-(u_d) .*(z);
     zeros(n,4),x,y,z,ones(n,1),-(v_d) .*(x),-(v_d) .*(y),-(v_d) .*(z)];

//Matrice B
B_cam_d = [u_d;v_d];

// Parametres du modele par calcul de la pseudo inverse
C_d = (pinv(A_cam_d))*(B_cam_d);

// Ecriture matricielle du modele global
Glob_d = [(C_d(1:4))';
     (C_d(5:8))';
     (C_d(9:11))',1];

// Reconstruction des pixels a partir des points 3D et du modele estime
B_cam_d_rec = Glob_d*[(x');(y');(z');ones(1,n)];
u_d_rec = B_cam_d_rec(1,:) ./B_cam_d_rec(3,:);
v_d_rec = B_cam_d_rec(2,:) ./B_cam_d_rec(3,:);

//----------------------------------------------------------------------------------------------------------------------------------- Reconstruction 3D
// Fonction 3D vers 2D
// Entree : modele camera + coordonnees 3D
// Sortie points 2D dans l image -> traduit les position 3D en 2D !
function [u,v]=camera_3D_2D(Glob,x_in,y_in,z_in)
//  n=size(x_in);
//B_cam_rec = Glob*[(x_in');(y_in');(z_in');ones(1,n)];
B_cam_rec = Glob*[(x_in');(y_in');(z_in');1];
u = B_cam_rec(1,:) ./B_cam_rec(3,:);
v = B_cam_rec(2,:) ./B_cam_rec(3,:);
endfunction


// Erreur de reconstruction
Err_cam_d = [u_d'-u_d_rec;v_d'-v_d_rec];

// Max Min Moyenne Ecart type
Max_Err_cam_d = max(Err_cam_d,'c');
Min_Err_cam_d = min(Err_cam_d,'c');
Moy_Err_cam_d = mean(Err_cam_d,'c');
Std_Err_cam_d = stdev(Err_cam_d,'c');


//----------------------------------------------------------------------------------------------------------------------------------- Reconstruction 2D
// Rayons visuels des deux cameras
for i=1:n,
 A(4*i-3,:)=[(Glob_g(1,1)-Glob_g(3,1)*u_g(i)) (Glob_g(1,2)-Glob_g(3,2)*u_g(i)) (Glob_g(1,3)-Glob_g(3,3)*u_g(i))];
 A(4*i-2,:)=[(Glob_g(2,1)-Glob_g(3,1)*v_g(i)) (Glob_g(2,2)-Glob_g(3,2)*v_g(i)) (Glob_g(2,3)-Glob_g(3,3)*v_g(i))];
 A(4*i-1,:)=[(Glob_d(1,1)-Glob_d(3,1)*u_d(i)) (Glob_d(1,2)-Glob_d(3,2)*u_d(i)) (Glob_d(1,3)-Glob_d(3,3)*u_d(i))];
 A(4*i,:)=[(Glob_d(2,1)-Glob_d(3,1)*v_d(i)) (Glob_d(2,2)-Glob_d(3,2)*v_d(i)) (Glob_d(2,3)-Glob_d(3,3)*v_d(i))];

 B(4*i-3)=[Glob_g(3,4)*u_g(i)-Glob_g(1,4)];
 B(4*i-2)=[Glob_g(3,4)*v_g(i)-Glob_g(2,4)] ;
 B(4*i-1)=[Glob_d(3,4)*u_d(i)-Glob_d(1,4)];
 B(4*i)=[Glob_d(3,4)*v_d(i)-Glob_d(2,4)] ;

// Points 3D reconstruits
 X(:,i)=pinv(A(4*i-3:4*i,:))*B(4*i-3:4*i);

end;

// Erreur de reconstruction
xyz=xyz';
Err_3D=X-xyz;

// Fonction 2D droite et gauche vers 3D
// Entree : modele camera droite et gauche + coordonnees 2D droite et gauche
// Sortie points 3D -> traduit les position 2D en 3D !
function [x,y,z]=camera_2D_3D(Globg, Globd,ug,vg,ud,vd)
 A(1,:)=[(Globg(1,1)-Globg(3,1)*ug) (Globg(1,2)-Globg(3,2)*ug) (Globg(1,3)-Globg(3,3)*ug)];
 A(2,:)=[(Globg(2,1)-Globg(3,1)*vg) (Globg(2,2)-Globg(3,2)*vg) (Globg(2,3)-Globg(3,3)*vg)];
 A(3,:)=[(Globd(1,1)-Globd(3,1)*ud) (Globd(1,2)-Globd(3,2)*ud) (Globd(1,3)-Globd(3,3)*ud)];
 A(4,:)=[(Globd(2,1)-Globd(3,1)*vd) (Globd(2,2)-Globd(3,2)*vd) (Globd(2,3)-Globd(3,3)*vd)];

 B(1)=[Globg(3,4)*ug-Globg(1,4)];
 B(2)=[Globg(3,4)*vg-Globg(2,4)] ;
 B(3)=[Globd(3,4)*ud-Globd(1,4)];
 B(4)=[Globd(3,4)*vd-Globd(2,4)] ;

// Points 3D reconstruits
 X=pinv(A)*B;
 x=X(1);
 y=X(2);
 z=X(3);
endfunction

fd = mopen('3D_initial.txt','w');
for i=1:n
mfprintf(fd,'%f\t%f\t%f\n',xyz(1,i),xyz(2,i),xyz(3,i));
end;
mclose(fd);
clear fd;
fd = mopen('3D_reconsrtuit.txt','w');
for i=1:n
mfprintf(fd,'%f\t%f\t%f\n',X(1,i),X(2,i),X(3,i));
end;
mclose(fd);
clear fd;
